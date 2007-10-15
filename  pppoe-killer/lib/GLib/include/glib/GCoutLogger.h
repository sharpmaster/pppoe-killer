#ifndef _GCOUTLOGGER_H
#define _GCOUTLOGGER_H
#include <glib/config.h>
#include <boost/thread/mutex.hpp>
#include <glib/GBaseLogger.h>

namespace glib
{
	class GLIB_EXPORT GCoutLogger : public glib::GBaseLogger
	{
	private:
		static boost::mutex m_LogcoutMutex;
	protected:
		virtual void Log(const char* logstr);
	public:
		GCoutLogger(int level);
		~GCoutLogger() {}
		GCoutLogger* clone() const;
	};
}

#endif

