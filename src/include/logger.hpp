#ifndef __FUSE3_7Z__LOGGER_HPP_
#define __FUSE3_7Z__LOGGER_HPP_

#include <stdarg.h>

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
