#include "fuse3.hpp"
#include "exception.hpp"
#include "logger.hpp"
#include "pattern.hpp"
#include "string.hpp"
#include "version.hpp"

#include <cstring>
#include <fstream>
#include <filesystem>
#include <map>
#include <memory>
#include <vector>

#define CMD_OPT(t, m) {t, offsetof(struct cmd_params_t, m), 1}

enum arg_status : int {
	ARG_DISCARD = 0,
	ARG_KEEP    = 1,
	ARG_ERROR   = -1,
};

struct Cache {
	using FileTree = std::map<sevenzip::Path, std::pair<sevenzip::IArchive::ci_iterator, sevenzip::Stat>>;
	sevenzip::IArchive*       arc;
	sevenzip::ExtractCallback ecb;
	FileTree                  tree;
};

static struct cmd_params_t {
	char*                    password = nullptr;
	char*                    passfile = nullptr;
	std::vector<std::string> cli_args;
	bool                     allow_other = false;
} cmd_params;

static const fuse_opt opts_spec[] = {CMD_OPT("--password=%s", password), CMD_OPT("-p %s", password),
									 CMD_OPT("--passfile=%s", passfile), FUSE_OPT_END};

static std::string cmd_password;

class Fuse::Params: public fuse_cmdline_opts {
public:
	~Params();
	Params(int argc, char** argv);

	void       print_usage();
	static int process_arg(Params* fuse, const char* arg, int key, struct fuse_args* outargs);

	fuse_args args;
};

Fuse::Params::~Params()
{
	free(mountpoint);
	fuse_opt_free_args(&args);
}

Fuse::Params::Params(int argc, char** argv)
{
	mountpoint = nullptr;
	args       = FUSE_ARGS_INIT(argc, argv);

	CheckResult(fuse_opt_parse(&args, &cmd_params, opts_spec, (fuse_opt_proc_t)&process_arg) != -1, "fuse_opt_parse");

	CheckResult(fuse_parse_cmdline(&args, this) == 0, "fuse_parse_cmdline");

	if (cmd_params.password) {
		cmd_password = cmd_params.password;
	} else if (cmd_params.passfile) {
		std::ifstream            passfile(cmd_params.passfile);
		std::istreambuf_iterator it(passfile);
		std::string              pass(it, {});
		cmd_password = pass;
	} else {
		auto password = std::getenv("FUSE3_P7ZIP_PASSWORD");
		if (password) {
			cmd_password = password;
		}
	}

	if (show_version) {
		printf("%s\n", PROJECT_VERSION);
		// printf("FUSE library version %s\n", fuse_pkgversion());
		// fuse_lowlevel_version();
		exit(0);
	} else if (show_help) {
		print_usage();
	} else if (cmd_params.cli_args.empty()) {
		fprintf(stderr, "error: no archive specified\n");
		exit(0);
	} else if (!mountpoint) {
		fprintf(stderr, "error: no mountpoint specified\n");
		exit(0);
	}
}

void Fuse::Params::print_usage()
{
	printf("usage: fuse3-7z [options] <archive> <mountpoint>\n\n");
	printf("Options:\n");
	printf("    -p <password>          provide a password for protected archives\n");
	printf("    --password <password>  provide a password for protected archives\n");
	printf("    --passfile <file>      provide a file with a password for protected archives\n");
	fuse_cmdline_help();
	fuse_lib_help(&args);
	exit(0);
}

int Fuse::Params::process_arg(Fuse::Params* fuse, const char* arg, int key, struct fuse_args* outargs)
{
	LogDebug("key: %d, arg: %s\n", key, arg);
	switch (key) {
		case FUSE_OPT_KEY_OPT:
			if (strcmp("allow_other", arg) == 0) {
				cmd_params.allow_other = true;
				return ARG_DISCARD;
			}
		case FUSE_OPT_KEY_NONOPT:
			cmd_params.cli_args.emplace_back(arg);
			if (cmd_params.cli_args.back().empty()) fuse->print_usage();
			if (cmd_params.cli_args.size() == 1) return ARG_DISCARD;

		default: return ARG_KEEP;
	}
}

class Fuse::Operations
	: public fuse_operations
	, public pattern::Uncopyable {
public:
	Operations();

	static void* cb_init(fuse_conn_info* conn, fuse_config* cfg);
	static void  cb_destroy(void*);
	static int   cb_getattr(const char* path, struct stat* stbuf, fuse_file_info* fi);
	static int   cb_open(const char* path, struct fuse_file_info* fi);
	static int   cb_release(const char* path, struct fuse_file_info* fi);
	static int   cb_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi);
	static int   cb_readdir(const char*, void*, fuse_fill_dir_t, off_t, fuse_file_info*, fuse_readdir_flags);
};

Fuse::Operations::Operations()
{
	std::memset(this, 0, sizeof(*this));
	init    = cb_init;
	getattr = cb_getattr;
	open    = cb_open;
	read    = cb_read;
	readdir = cb_readdir;
	release = cb_release;
}

void* Fuse::Operations::cb_init(fuse_conn_info*, fuse_config* cfg)
{
	// cfg->entry_timeout    = NO_TIMEOUT;
	// cfg->attr_timeout     = NO_TIMEOUT;
	// cfg->negative_timeout = 0;

	auto ctx = fuse_get_context();

	return ctx->private_data;
}

void Fuse::Operations::cb_destroy(void*) {}

int Fuse::Operations::cb_getattr(const char* path, struct stat* stbuf, fuse_file_info*)
{
	std::memset(stbuf, 0, sizeof(struct stat));

	Cache* cache = static_cast<Cache*>(fuse_get_context()->private_data);
	auto   el    = cache->tree.find(path);
	if (el == cache->tree.end()) return -ENOENT;

	*stbuf = el->second.second;
	return 0;
}

int Fuse::Operations::cb_open(const char* path, fuse_file_info* fi)
{
	LogDebug("fuse3 open: %s", path);
	Cache* cache = static_cast<Cache*>(fuse_get_context()->private_data);
	auto   el    = cache->tree.find(path);
	if (el == cache->tree.end()) return -ENOENT;

	auto override_max_size = std::getenv("FUSE3_P7ZIP_MAX_OPEN_SIZE");
	if (override_max_size) {
		auto size = std::strtoll(override_max_size, nullptr, 10);
		if (el->second.second.st_size > size) return -E2BIG;
	}

	auto tmpfile = el->second.first.extract(cache->ecb).release();
	fi->fh       = reinterpret_cast<uint64_t>(tmpfile);
	LogDebug("fuse3 open fh: %p", fi->fh);

	return 0;
}

int Fuse::Operations::cb_release(const char*, struct fuse_file_info* fi)
{
	auto file = reinterpret_cast<sevenzip::IFile*>(fi->fh);
	delete file;
	return 0;
}

int Fuse::Operations::cb_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
	LogDebug("fuse3 read: %s", path);
	auto file = reinterpret_cast<sevenzip::IFile*>(fi->fh);
	file->seek(offset, sevenzip::IFile::Whence::set);
	return file->read(buf, size);
}

int Fuse::Operations::cb_readdir(const char* p, void* buf, fuse_fill_dir_t filler, off_t, fuse_file_info*,
								 fuse_readdir_flags)
{
	Cache* cache = static_cast<Cache*>(fuse_get_context()->private_data);
	if (strcmp("/", p) != 0 && !cache->tree.contains(p)) return -ENOTDIR;

	auto path = sevenzip::Path(p);

	filler(buf, ".", NULL, 0, FUSE_FILL_DIR_PLUS);
	filler(buf, "..", NULL, 0, FUSE_FILL_DIR_PLUS);
	for (auto& it : cache->tree) {
		auto parent = it.first.parent_path();
		auto name   = it.first.filename();
		if (path == parent && !name.empty()) {
			filler(buf, name.c_str(), &it.second.second, 0, FUSE_FILL_DIR_PLUS);
		}
	}
	return 0;
}

Fuse::~Fuse()
{
	if (fuse) fuse_destroy(fuse);
}

Fuse::Fuse(int argc, char** argv)
	: fuse()
	, _params(std::make_unique<Params>(argc, argv))
	, _operations(std::make_unique<Operations>())
{}

const std::string& Fuse::path() const
{
	return cmd_params.cli_args[0];
}

std::string Fuse::password() const
{
	return cmd_password;
}

ssize_t Fuse::execute(sevenzip::IArchive* arc)
{
	static auto cache     = Cache{arc, sevenzip::ExtractCallback(password()), Cache::FileTree()};
	auto        root      = sevenzip::Path("/");
	auto        root_stat = arc->stat();

	root_stat.st_mode = (root_stat.st_mode & ~S_IFMT) | S_IFDIR | S_IXUSR | S_IXGRP;
	if (cmd_params.allow_other) root_stat.st_mode |= S_IROTH | S_IXOTH;

	cache.tree.emplace(root, std::make_pair(arc->end(), root_stat));
	for (auto it = arc->begin(); it != arc->end(); ++it) {
		//log().printf("it: '%s'\n", it.path().c_str());
		auto path = root / it.path();
		auto name = sevenzip::Path(root == path && it.name().empty() ? arc->proposed_name() : it.name());
		if (!it.extension().empty()) name += "." + it.extension();

		if (!name.empty()) path /= name;

		auto file_stat = arc->stat();
		file_stat.st_mode &= ~S_IFMT;
		file_stat.st_size = it.size();
		file_stat.st_atim = it.atime();
		file_stat.st_mtim = it.mtime();
		file_stat.st_ctim = it.ctime();

		if (it.is_dir()) {
			file_stat.st_mode |= S_IFDIR | 0775;
		} else {
			file_stat.st_mode |= S_IFREG | 0664;
		}
		//log().printf("path: '%s'\n", path.c_str());
		cache.tree.insert_or_assign(path, std::make_pair(it, file_stat));
		for (auto p = path.parent_path(); !p.empty() && p != root; p = p.parent_path()) {
			if (cache.tree.contains(p)) break;
			//log().printf("sub: '%s'\n", p.c_str());
			file_stat.st_size = it.size();
			file_stat.st_mode &= ~S_IFMT;
			file_stat.st_mode |= S_IFDIR | 0775;
			cache.tree.emplace(p, std::make_pair(arc->end(), file_stat));
		}
	}

	fuse = fuse_new(&_params->args, _operations.get(), sizeof(fuse_operations), &cache);

	if (fuse_mount(fuse, _params->mountpoint) != 0) {
		exit(1);
	}

	if (fuse_daemonize(_params->foreground) != 0) {
		exit(1);
	}

	auto se = fuse_get_session(fuse);
	if (fuse_set_signal_handlers(se) != 0) {
		exit(1);
	}

	auto ret = fuse_loop(fuse);

	fuse_remove_signal_handlers(se);

	fuse_unmount(fuse);

	return ret;
}
