#ifndef _GNULLLOGGER_H
#define _GNULLLOGGER_H
#include <glib/config.h>
#include <glib/GBaseLogger.h>

namespace glib
{
	class GLIB_EXPORT GNullLogger : public GBaseLogger
	{
	public:
		GNullLogger() : GBaseLogger(GBaseLogger::FATAL_LEVEL) {}
		~GNullLogger() {}
	protected:
		virtual void Log(const char* logstr) {}
		GNullLogger* clone() const { return new GNullLogger();}
	};
}

#endif

