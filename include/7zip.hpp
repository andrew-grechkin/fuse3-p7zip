#ifndef __FUSE3_7Z__7ZIP_HPP_
#define __FUSE3_7Z__7ZIP_HPP_

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>

namespace sevenzip {
	class ILib;
	class IFormat;
	class IMethod;
	class IArchive;
	class IOpenCallback;
	class IArchiveExtractor;
	class IArchiveUpdator;
	class IExtractCallback;
	class IUpdateCallback;
	class ImplArchive;
	class IFile;

	struct Guid;

	using Stat       = struct stat;
	using Time       = struct timespec;
	using Path       = std::filesystem::path;
	using Lib        = std::unique_ptr<ILib>;
	using Format     = std::shared_ptr<IFormat>;
	using Method     = std::unique_ptr<IMethod>;
	using Archive    = std::unique_ptr<IArchive>;
	using Formats    = std::vector<Format>;
	using Methods    = std::vector<Method>;
	using Extensions = std::vector<std::string>;
	using Size       = uint64_t;
	using flags_type = ssize_t;
	using File       = std::unique_ptr<IFile>;

	Lib open(const Path& path);

	class ILib {
	public:
		virtual ~ILib() noexcept = default;

		virtual const Path&    path() const noexcept    = 0;
		virtual const Formats& formats() const noexcept = 0;
		virtual const Methods& methods() const noexcept = 0;

		virtual Archive open(const Path& path, const IOpenCallback& callback, flags_type flags = 0) const = 0;
	};

	class IFormat {
	public:
		virtual ~IFormat() noexcept = default;

		virtual std::string       name() const noexcept                 = 0;
		virtual const Extensions& default_extension() const noexcept    = 0;
		virtual const Extensions& additional_extension() const noexcept = 0;
		virtual const Guid*       guid() const noexcept                 = 0;
		virtual bool              updatable() const noexcept            = 0;

		bool operator==(const std::string& rhs) const noexcept
		{
			return name() == rhs;
		}
		bool operator<(const std::string& rhs) const noexcept
		{
			return name() < rhs;
		}
	};

	class IMethod {
	public:
		virtual ~IMethod() noexcept = default;

		virtual std::string name() const noexcept = 0;
	};

	class IOpenCallback {
	public:
		virtual ~IOpenCallback() noexcept = default;

		virtual std::string request_password() const = 0;
	};

	class IExtractCallback: public IOpenCallback {
	public:
		virtual bool notify_progress(uint64_t current, uint64_t of) const = 0;
	};

	class EmptyOpenCallback: public IOpenCallback {
		std::string request_password() const override
		{
			return std::string();
		}
	};

	class EmptyExtractCallback: public IExtractCallback {
		std::string request_password() const override
		{
			return std::string();
		}
		bool notify_progress(uint64_t, uint64_t) const override
		{
			return true;
		}
	};

	class IFile {
	public:
		enum Whence : uint8_t { set = 0, cur = 1, end = 2 };

		virtual ~IFile() noexcept = default;

		virtual uint64_t read(void* data, uint64_t size)        = 0;
		virtual uint64_t write(const void* data, uint64_t size) = 0;
		virtual uint64_t seek(uint64_t pos, Whence whence)      = 0;
	};

	class IArchive {
	public:
		struct ci_iterator;

		using iterator       = ci_iterator;
		using const_iterator = ci_iterator;

		virtual ~IArchive() noexcept = default;

		virtual const Path&   path() const noexcept          = 0;
		virtual const Stat&   stat() const noexcept          = 0;
		virtual const Format& format() const noexcept        = 0;
		virtual std::string   proposed_name() const noexcept = 0;

		virtual bool   empty() const noexcept = 0;
		virtual size_t size() const noexcept  = 0;

		virtual const_iterator begin() const noexcept = 0;
		virtual const_iterator end() const noexcept   = 0;
		virtual const_iterator at(size_t index) const = 0;
		virtual flags_type     flags() const noexcept = 0;
	};

	struct IArchive::ci_iterator {
		~ci_iterator() noexcept;
		ci_iterator(const ci_iterator& other);
		ci_iterator(ci_iterator&& other) noexcept;

		ci_iterator& operator=(const ci_iterator& other);
		ci_iterator& operator=(ci_iterator&& other) noexcept;
		void         swap(ci_iterator& other) noexcept;

		ci_iterator& operator++();
		ci_iterator  operator++(int);

		std::string path() const;
		std::string name() const;
		std::string extension() const;
		std::string comment() const;
		Size        size() const;
		Size        size_packed() const;
		Time        atime() const;
		Time        mtime() const;
		Time        ctime() const;

		bool is_file() const;
		bool is_dir() const;

		File extract(const IExtractCallback& callback = EmptyExtractCallback()) const;

		size_t get_props_count() const;

		bool operator==(const ci_iterator& other) const noexcept;
		bool operator!=(const ci_iterator& other) const noexcept;

	private:
		struct Impl;
		using Impl_type = std::unique_ptr<Impl>;
		Impl_type impl;

		ci_iterator(Impl_type&& impl) noexcept;

		friend class ImplArchive;
	};

	class IArchiveExtractor {
	public:
		virtual ~IArchiveExtractor() noexcept = default;

		virtual void execute(bool test) = 0;
	};
} // namespace sevenzip

#endif
