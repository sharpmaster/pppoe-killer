#include <glib/GCoutLogger.h>
#include <iostream>
#include <glib/GTime.h>

using namespace glib;

boost::mutex GCoutLogger::m_LogcoutMutex;

GCoutLogger::GCoutLogger(int level) : GBaseLogger(level)
{
}

GCoutLogger* GCoutLogger::clone() const
{
	return new GCoutLogger(m_level);
}

void GCoutLogger::Log(const char* logstr)
{
	std::string timestr = GTime::GetTimeString();
	boost::mutex::scoped_lock lock(m_LogcoutMutex);
	std::cout << timestr.c_str() << "  " << logstr << std::endl;
}

