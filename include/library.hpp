#ifndef __FUSE3_7Z__LIBRARY_HPP_
#define __FUSE3_7Z__LIBRARY_HPP_

#include <filesystem>
#include <memory>
#include <string>

namespace library {
	class IDynamic;

	using Dynamic    = std::unique_ptr<IDynamic>;
	using Path       = std::filesystem::path;
	using Symbol     = void*;
	using flags_type = ssize_t;

	Dynamic open(const Path& path, flags_type flags = flags_type());

	class IDynamic {
	public:
		virtual ~IDynamic() noexcept = default;

		virtual const Path& path() const noexcept = 0;

		virtual Symbol get_sym(const char* name) const             = 0;
		virtual Symbol get_sym_nt(const char* name) const noexcept = 0;
	};
} // namespace library

#endif
