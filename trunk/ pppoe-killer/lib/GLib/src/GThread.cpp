#include <glib/GThread.h>
#include <boost/bind.hpp>
#include <boost/thread/xtime.hpp>

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
	boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC);
	xt.sec += millis/1000;
	xt.nsec += ((millis%1000)*1000000);

	boost::thread::sleep(xt);
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

