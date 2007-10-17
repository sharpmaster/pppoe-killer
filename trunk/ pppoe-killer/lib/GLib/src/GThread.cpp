#include <glib/GThread.h>
#include <boost/bind.hpp>
#include <boost/thread/xtime.hpp>

#ifdef WIN32
#include <windows.h>
#else
#include <time.h>
#endif

using namespace glib;

GThread::GThread()
{
	m_stopping = false;
	m_running = false;
	m_thread = NULL;
}

GThread::~GThread()
{
	if(m_thread != NULL)
	{
		delete m_thread;
	}
	m_thread = NULL;
}

void GThread::run0()
{
	this->run();
	m_running = false;
}

void GThread::start()
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
		m_thread = new boost::thread(boost::bind(&GThread::run0, this));
	}
}

void GThread::stop()
{
	m_stopping = true;
}

bool GThread::isAlive()
{
	return m_running;
}

void GThread::sleep(unsigned int millis)
{
#ifdef WIN32
	Sleep(millis);
#else
	struct timespec t;

	if(millis > 1000)
	{
		t.tv_sec = millis/1000;
		t.tv_nsec = (millis%1000)*1000000;
	}
	else
	{
		t.tv_sec = 0;
		t.tv_nsec = millis*1000000;
	}
	nanosleep(&t, NULL);
#endif
}

void GThread::join()
{
	if(m_thread != NULL)
		m_thread->join();
}

bool GThread::IsStopping()
{
	return m_stopping;
}

