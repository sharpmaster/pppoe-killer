#ifndef _GTHREAD_H
#define _GTHREAD_H
#include <glib/config.h>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>

namespace glib
{
	class GLIB_EXPORT GThread : boost::noncopyable
	{
	private:
		boost::thread *m_thread;
		bool m_stopping;
		bool m_running;

		void run0();
	protected:
		bool IsStopping();
	public:
		GThread();
		virtual ~GThread();
		void start();
		void stop();
		void waitStop();
		bool timedStop(unsigned int millis);
		bool isAlive();
		static void sleep(unsigned int millis);
		void join();
		
		virtual void run() = 0;
	};
}

#endif

