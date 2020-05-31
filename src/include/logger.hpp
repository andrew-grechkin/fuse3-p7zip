#ifndef __FUSE3_7Z__LOGGER_HPP_
#define __FUSE3_7Z__LOGGER_HPP_

#include <stdarg.h>

#ifdef NDEBUG
#	define LogTrace()
#	define LogDebug(format, ...)
#else
#	define LogTrace()            log().printf("%s:%d %s", __FILE__, __LINE__, __PRETTY_FUNCTION__)
#	define LogDebug(format, ...) log().printf(format, ##__VA_ARGS__)
#endif

class Log {
public:
	void printf(const char* format, ...) noexcept;

private:
	~Log();
	Log();

	friend Log& log();
};

Log& log();

#endif
