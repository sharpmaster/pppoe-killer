#ifndef _HIPPOLIB_THREAD_H
#define _HIPPOLIB_THREAD_H

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>

#ifdef WIN32
#include <windows.h>
#endif

namespace hippolib
{
	class thread : boost::noncopyable
	{
	public:
		thread() : m_stopping(false), m_running(false), m_thread(NULL)
		{
		}
		
		virtual ~thread()
		{
			if(m_thread != NULL)
			{
				delete m_thread;
			}
			m_thread = NULL;
		}
		
		void start()
		{
			if(m_running == false || m_thread == NULL)
			{
				if(m_thread != NULL)
				{
					delete m_thread;
					m_thread = NULL;
				}

				m_running = true;
				m_stopping = false;
				m_thread = new boost::thread(boost::bind(&hippolib::thread::run0, this));
			}
		}
		
		void stop()
		{
			m_stopping = true;
		}
		
		void waitStop()
		{
			stop();
			while(this->isAlive())
				hippolib::thread::sleep(100);
		}
		
		bool timedStop(unsigned int millis)
		{
			unsigned int elapsed = 0;

			stop();
			while(this->isAlive() && elapsed < millis)
			{
				hippolib::thread::sleep(100);
				elapsed += 100;
			}

			return !(this->isAlive());
		}
		
		bool isAlive() const
		{
			return m_running;
		}
		
		static void sleep(unsigned int millis)
		{
			#ifdef WIN32
			Sleep(millis);
			#else
			usleep(millis);
			#endif
		}
		
		void join()
		{
			if(m_thread != NULL)
				m_thread->join();
		}
		
		virtual void run() = 0;

	protected:
		bool IsStopping()
		{
			return m_stopping;
		}
		
	private:
		
		bool m_stopping;
		bool m_running;
		boost::thread *m_thread;

		void run0()
		{
			this->run();
			m_running = false;
		}
	};
}

#endif

