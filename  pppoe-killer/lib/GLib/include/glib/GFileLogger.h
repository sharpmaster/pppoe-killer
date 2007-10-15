#ifndef _GFILELOGGER_H
#define _GFILELOGGER_H
#include <glib/config.h>
#include <boost/thread/mutex.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/filesystem/path.hpp>
#include <glib/GBaseLogger.h>

namespace glib
{
	class GLIB_EXPORT GFileLogger : public glib::GBaseLogger
	{
	private:
		std::string m_filename;
		int m_filesize;

		static boost::mutex ms_mutexofmutex;
		static boost::ptr_map<boost::filesystem::path, boost::mutex> ms_LogFileMutexes;
	protected:
		virtual void Log(const char* logstr);
	public:
		GFileLogger(int level, const std::string & filename, int filesize);
		~GFileLogger() {}
		GFileLogger* clone() const;
	};
}

#endif

