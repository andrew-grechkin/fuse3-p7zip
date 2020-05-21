#include "exception.hpp"
#include "library-impl.hpp"

#include <dlfcn.h>

#ifdef NDEBUG
void ThrowLibraryError(const char* str)
{
	throw std::runtime_error(std::string(str) + " " + dlerror());
}
#else
void ThrowLibraryError(const char* str, const char* file, size_t line, const char* func)
{
	throw std::runtime_error(std::string(str) + " " + dlerror());
}
#endif

namespace library {
	Dynamic open(const Path& path, flags_type flags)
	{
		return std::make_unique<ImplDynamic>(path, flags);
	}

	ImplDynamic::~ImplDynamic() noexcept
	{
		dlclose(_hnd);
	}

	ImplDynamic::ImplDynamic(const Path& path, flags_type flags)
		: _path(path)
		, _flags(flags | RTLD_NOW)
		, _hnd(dlopen(path.c_str(), _flags))
	{
		CheckPointer(_hnd, ThrowLibraryError, "dlopen");
	}

	const Path& ImplDynamic::path() const noexcept
	{
		//Dl_info info{};
		//if (dladdr(m_hnd, &info) && info.dli_fname) {
		//return info.dli_fname;
		//}
		return _path;
	}

	Symbol ImplDynamic::get_sym(const char* name) const
	{
		auto ptr = get_sym_nt(name);
		CheckPointer(ptr, ThrowLibraryError, "dlsym");
		return ptr;
	}

	Symbol ImplDynamic::get_sym_nt(const char* name) const noexcept
	{
		return dlsym(_hnd, name);
	}
} // namespace library
