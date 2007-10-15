#include <glib/GFileLogger.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <glib/GTime.h>

using namespace glib;
using namespace std;
namespace fs = boost::filesystem;

boost::mutex GFileLogger::ms_mutexofmutex;
boost::ptr_map<fs::path, boost::mutex> GFileLogger::ms_LogFileMutexes;

GFileLogger::GFileLogger(int level, const string & filename, int filesize)
	: GBaseLogger(level)
{
	m_filename = filename;
	m_filesize = filesize;
}

GFileLogger* GFileLogger::clone() const
{
	return new GFileLogger(m_level, m_filename, m_filesize);
}

void GFileLogger::Log(const char* logstr)
{
	boost::mutex::scoped_lock *lock = NULL;
	boost::mutex::scoped_lock *mutexlock = NULL;
	fs::path filepath(m_filename);
	fs::fstream filestream;

	mutexlock = new boost::mutex::scoped_lock(ms_mutexofmutex);
	boost::ptr_map<fs::path, boost::mutex>::iterator mi = ms_LogFileMutexes.find(filepath);
	if(mi == ms_LogFileMutexes.end())
	{
		boost::mutex *filemutex = new boost::mutex;
		ms_LogFileMutexes.insert(filepath, filemutex);
	}
	delete mutexlock;

	lock = new boost::mutex::scoped_lock(ms_LogFileMutexes[filepath]);
	filestream.open(filepath, ios_base::out | ios_base::app);
	if(filestream.good() == false)
	{
		delete lock;
		return;
	}
	 
	if(m_filesize > 0)
	{
		filestream.seekp(0, ios_base::end);
		int length = (int)filestream.tellp();
		// Replace the log file
		if (length > m_filesize) {
			filestream.close();
			filestream.open(filepath, ios_base::out | ios_base::trunc);
			if(filestream.good() == false)
			{
				delete lock;
				return;
			}
		}
	}

	std::string timestr = GTime::GetTimeString();
	filestream << timestr << "  " << logstr << endl;
	filestream.close();
	delete lock;
}

