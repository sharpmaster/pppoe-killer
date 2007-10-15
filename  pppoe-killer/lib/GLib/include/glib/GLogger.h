#ifndef _GLOGGER_H
#define _GLOGGER_H
#include <glib/config.h>
#include <string>
#include <map>
#include <glib/GProperties.h>
#include <glib/GFileLogger.h>
#include <glib/GCoutLogger.h>
#include <glib/GNullLogger.h>

namespace glib
{
	class GLIB_EXPORT GLogger
	{
	private:
		static bool m_configured;
		static std::map<std::string, std::map<std::string, std::string> > m_config;
	public:
		static bool configure(const GProperties & config);
		static int getLevel(const std::string & level);
		static glib::GBaseLogger* getLogger(const std::string & name);
	};
}

#endif

