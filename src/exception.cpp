#include "exception.hpp"
#include <string>

#include "CPP/Common/MyWindows.h"

namespace exception {
	void HiddenFunctions::ThrowErrnoFunc(const std::string& str)
	{
		throw std::runtime_error(str + " [" + std::to_string(errno) + "]: " + strerror(errno));
	}

	void HiddenFunctions::ThrowErrnoFunc(const std::string& str, const char* file, size_t line, const char* func)
	{
		throw std::runtime_error(str + " [" + std::to_string(errno) + "]: " + strerror(errno) + "\n  " + func + ":" +
								 std::to_string(line) + "\n  " + file);
	}

	void HiddenFunctions::ThrowResultFunc(const std::string& str)
	{
		throw std::runtime_error(str);
	}

	void HiddenFunctions::ThrowResultFunc(const std::string& str, const char* file, size_t line, const char* func)
	{
		throw std::runtime_error(str + "\n  " + func + ":" + std::to_string(line) + "\n  " + file);
	}

	HRESULT HiddenFunctions::CheckComFunc(HRESULT res, const std::string& str)
	{
		if (FAILED(res)) throw std::runtime_error(str);
		return res;
	}

	HRESULT HiddenFunctions::CheckComFunc(HRESULT res, const std::string& str, const char* file, size_t line,
										  const char* func)
	{
		if (FAILED(res)) throw std::runtime_error(str + "\n  " + func + ":" + std::to_string(line) + "\n  " + file);
		return res;
	}
} // namespace exception
