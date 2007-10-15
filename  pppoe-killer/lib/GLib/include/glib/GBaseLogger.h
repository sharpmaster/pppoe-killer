#ifndef _GBASELOGGER_H
#define _GBASELOGGER_H
#include <glib/config.h>
#include <string>
#include <map>
#include <boost/utility.hpp>

#define GTRACE(logger) if((logger).isTraceEnabled())(logger).forcedLog
#define GDEBUG(logger) if((logger).isDebugEnabled())(logger).forcedLog
#define GINFO(logger) if((logger).isInfoEnabled())(logger).forcedLog
#define GWARN(logger) if((logger).isWarnEnabled())(logger).forcedLog
#define GERROR(logger) if((logger).isErrorEnabled())(logger).forcedLog
#define GFATAL(logger) if((logger).isFatalEnabled())(logger).forcedLog

namespace glib
{
	class GLIB_EXPORT GBaseLogger : boost::noncopyable
	{
	protected:
		int m_level;
		virtual void Log(const char*  logstr) = 0;
	public:
		
		enum
		{
			TRACE_LEVEL = 0,
			DEBUG_LEVEL,
			INFO_LEVEL,
			WARN_LEVEL,
			ERROR_LEVEL,
			FATAL_LEVEL
		};
				
		GBaseLogger(int level);
		virtual ~GBaseLogger() {}

		bool isTraceEnabled() const;
		bool isDebugEnabled() const;
		bool isInfoEnabled() const;
		bool isWarnEnabled() const;
		bool isErrorEnabled() const;
		bool isFatalEnabled() const;
		void forcedLog(const char* logstr, ...);
		int getLevel() const;
		void setLevel(int level);
		
		virtual GBaseLogger* clone() const = 0;
	};
}

#endif

