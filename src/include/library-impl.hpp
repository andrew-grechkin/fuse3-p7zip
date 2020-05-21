#ifndef __FUSE3_7Z__LIBRARY_IMPL_HPP_
#define __FUSE3_7Z__LIBRARY_IMPL_HPP_

#include "library.hpp"
#include "pattern.hpp"

#define DEFINE_SYM(prefix, name)         prefix##name name;
#define GET_LIBRARY_SYM(prefix, name)    name = (prefix##name)get_sym(#name);
#define GET_LIBRARY_SYM_NT(prefix, name) name = (prefix##name)get_sym_nt(#name);

namespace library {
	using Handle = void*;

	class ImplDynamic
		: public IDynamic
		, public pattern::Uncopyable {
		using this_type = ImplDynamic;

	public:
		~ImplDynamic() noexcept;
		ImplDynamic(const Path& path, flags_type flags = flags_type());

		const Path& path() const noexcept override;

		Symbol get_sym(const char* name) const override;
		Symbol get_sym_nt(const char* name) const noexcept override;

	private:
		Path       _path;
		flags_type _flags;
		Handle     _hnd;
	};
} // namespace library

#endif
