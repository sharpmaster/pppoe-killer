#include <glib/GBaseLogger.h>
#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#define vsnprintf _vsnprintf
#endif
#include <stdarg.h>
#include <vector>

using namespace std;
using namespace glib;

GBaseLogger::GBaseLogger(int level)
{
	if(level >= DEBUG_LEVEL && level <= FATAL_LEVEL)
		m_level = level;
	else
		m_level = DEBUG_LEVEL;
}

bool GBaseLogger::isTraceEnabled() const
{
	if(m_level <= TRACE_LEVEL)
		return true;
	return false;
}

bool GBaseLogger::isDebugEnabled() const
{
	if(m_level <= DEBUG_LEVEL)
		return true;
	return false;
}

bool GBaseLogger::isInfoEnabled() const
{
	if(m_level <= INFO_LEVEL)
		return true;
	return false;
}

bool GBaseLogger::isWarnEnabled() const
{
	if(m_level <= WARN_LEVEL)
		return true;
	return false;
}

bool GBaseLogger::isErrorEnabled() const
{
	if(m_level <= ERROR_LEVEL)
		return true;
	return false;
}

bool GBaseLogger::isFatalEnabled() const
{
	if(m_level <= FATAL_LEVEL)
		return true;
	return false;
}

int GBaseLogger::getLevel() const
{
	return m_level;
}

void GBaseLogger::setLevel(int level)
{
	m_level = level;
}

void GBaseLogger::forcedLog(const char* logstr, ...)
{
	va_list args;
	char buffer[4096];
	
	va_start(args, logstr);
	if(vsnprintf(buffer, 4096, logstr, args) == -1)
		buffer[4095] = 0;
	va_end(args);
	
	Log(buffer);
}


