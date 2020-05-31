#include "7zip.hpp"
#include "library.hpp"
#include "fuse3.hpp"
#include "logger.hpp"

// TODO
// verbose --version

int main(int argc, char** argv, char** env)
try {
	auto override_library = std::getenv("FUSE3_P7ZIP_LIBRARY");
	auto lib_path         = sevenzip::Path(override_library ? override_library : "/usr/lib/p7zip/7z.so");

	// preload Rar codecs (this also can be done with LD_PRELOAD)
	library::Dynamic rar_codec;
	auto             rar_path = lib_path.parent_path() / "Codecs" / "Rar.so";
	if (std::filesystem::exists(rar_path)) {
		library::open(rar_path).swap(rar_codec);
	}

	auto lib = sevenzip::open(lib_path);

	Fuse loop(argc, argv);
	log().printf("open archive: %s", loop.path().c_str());
	auto arc = lib->open(loop.path(), sevenzip::EmptyOpenCallback());

	return loop.execute(arc.get());
} catch (std::exception& e) {
	fprintf(stderr, "Critical error:\n  %s\n", e.what());
	return 1;
}
