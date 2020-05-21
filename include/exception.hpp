#ifndef __FUSE3_7Z__EXCEPTION_HPP_
#define __FUSE3_7Z__EXCEPTION_HPP_

#include <exception>
#include <stdexcept>

#ifdef NDEBUG
#	define CheckErrno(arg, str)                                                                                       \
		({                                                                                                             \
			if (!(arg)) exception::HiddenFunctions::ThrowErrnoFunc((str));                                             \
		})
#	define CheckPointer(arg, hnd, str)                                                                                \
		({                                                                                                             \
			if ((arg) == nullptr) hnd((str));                                                                          \
		})
#	define CheckResult(arg, str)                                                                                      \
		({                                                                                                             \
			if (!(arg)) exception::HiddenFunctions::ThrowResultFunc((str));                                            \
		})
#	define CheckCom(arg, str) (exception::HiddenFunctions::CheckComFunc((arg), (str)))
#else
#	define THROW_PLACE __FILE__, __LINE__, __PRETTY_FUNCTION__
#	define CheckErrno(arg, str)                                                                                       \
		({                                                                                                             \
			if (!(arg)) exception::HiddenFunctions::ThrowErrnoFunc((str), THROW_PLACE);                                \
		})
#	define CheckPointer(arg, hnd, str)                                                                                \
		({                                                                                                             \
			if ((arg) == nullptr) hnd((str), THROW_PLACE);                                                             \
		})
#	define CheckResult(arg, str)                                                                                      \
		({                                                                                                             \
			if (!(arg)) exception::HiddenFunctions::ThrowResultFunc((str), THROW_PLACE);                               \
		})
#	define CheckCom(arg, str) (exception::HiddenFunctions::CheckComFunc((arg), (str), THROW_PLACE))
#endif

namespace exception {
	//using HANDLE  = size_t;
	using HRESULT = int32_t;

	struct HiddenFunctions {
		static void ThrowErrnoFunc(const std::string& str);
		static void ThrowErrnoFunc(const std::string& str, const char* file, size_t line, const char* func);

		static void ThrowResultFunc(const std::string& str);
		static void ThrowResultFunc(const std::string& str, const char* file, size_t line, const char* func);

		static HRESULT CheckComFunc(HRESULT res, const std::string& str);
		static HRESULT CheckComFunc(HRESULT res, const std::string& str, const char* file, size_t line,
									const char* func);
	};
} // namespace exception

#endif
