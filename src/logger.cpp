#include "logger.hpp"

#include <syslog.h>
#include <stdarg.h>

Log::~Log()
{
	closelog();
}

Log::Log()
{
	openlog("fuse3-p7zip", LOG_CONS | LOG_NDELAY, LOG_USER);
}

void Log::printf(const char* format, ...) noexcept
{
	va_list vl;
	va_start(vl, format);
	vsyslog(LOG_NOTICE, format, vl);
	va_end(vl);
}

Log& log()
{
	static Log instance;
	return instance;
}
