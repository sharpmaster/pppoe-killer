#include <glib/GLogger.h>
#include <vector>
#include <glib/GProperties.h>

using namespace std;
using namespace glib;

bool GLogger::m_configured = false;
map<string, map<string, string> > GLogger::m_config;

bool GLogger::configure(const GProperties & config)
{
	m_configured = true;
	
	string loggers = config.get("loggers", "");
	vector<string> logger_names;
	string::size_type oldidx, newidx;
	if(loggers != "")
	{
		oldidx = 0;
		//cout << loggers << endl;
		while(true)
		{
			string name;

			if((newidx = loggers.find_first_of(',', oldidx)) == string::npos)
			{
				if(oldidx < loggers.size()-1)
				{
					name = loggers.substr(oldidx);
					logger_names.push_back(name);
				}
				break;
			}

			name = loggers.substr(oldidx, newidx-oldidx);
			logger_names.push_back(name);
			oldidx = newidx + 1;
		}
	}

	map<string, string> table;
	map<string, string> roottable;
	
	/**
	* Search the config of root logger
	* If not found, set the default values
	*/
	string key, value;
	value = config.get("root.logger", "");
	if(value == "")
		roottable.insert(make_pair("logger", "console"));
	else
		roottable.insert(make_pair("logger", value));

	if(roottable["logger"] == "file")
	{
		value = config.get("root.logger.filename", "messages.log");
		roottable.insert(make_pair("logger.filename", value));
		value = config.get("root.logger.filesize", "1048579");
		roottable.insert(make_pair("logger.filesize", value));
	}

	value = config.get("root.level", "");
	if(value == "")
		roottable.insert(make_pair("level", "warn"));
	else
		roottable.insert(make_pair("level", value));

	m_config.insert(make_pair("root", roottable));

	for(string::size_type i = 0; i < logger_names.size(); i++)
	{
		string name = logger_names[i];

		table.clear();
		
		key = name + ".logger";
		value = config.get(key, "");
		if(value == "")
			table.insert(make_pair("logger", roottable["logger"]));
		else
			table.insert(make_pair("logger", value));
		if(table["logger"] == "file")
		{
			key = name + ".logger.filename";
			value = config.get(key, "");
			if(value == "")
				table.insert(make_pair("logger.filename", roottable["logger.filename"]));
			else
				table.insert(make_pair("logger.filename", value));
			key = name + ".logger.filesize";
			value = config.get(key, "");
			if(value == "")
				table.insert(make_pair("logger.filesize", roottable["logger.filesize"]));
			else
				table.insert(make_pair("logger.filesize", value));
		}
		
		key = name + ".level";
		value = config.get(key, "");
		if(value == "")
			table.insert(make_pair("level", roottable["level"]));
		else
			table.insert(make_pair("level", value));

		m_config.insert(make_pair(name, table));
	}

	return true;
}

GBaseLogger* GLogger::getLogger(const string & name)
{
	GBaseLogger *ret = NULL;
	map<string, map<string, string> >::iterator ci;

	/**
	* If GLogger is not configured, we return a console logger with debug level
	* Since configuration for GLogger is mostly not avaliable in test programs, 
	* console loggers are the most proper loggers
	*/
	if(m_configured == false)
	{
		ret = new GCoutLogger(getLevel("debug"));
		return ret;
	}
	
	ci = m_config.find(name);
	if(ci == m_config.end())
	{
		ret = new GNullLogger();
		return ret;
	}

	map<string, string> table = ci->second;
	if(table["logger"] == "console")
	{
		ret = new GCoutLogger(getLevel(table["level"]));
	}
	else if(table["logger"] == "file")
	{
		ret = new GFileLogger(getLevel(table["level"]),
					table["logger.filename"], atoi(table["logger.filesize"].c_str()));
	}
	else
		ret = new GNullLogger();
	
	return ret;
}


int GLogger::getLevel(const string & level)
{
	if(level == "debug")
		return GBaseLogger::DEBUG_LEVEL;
	else if(level == "info")
		return GBaseLogger::INFO_LEVEL;
	else if(level == "warn")
		return GBaseLogger::WARN_LEVEL;
	else if(level == "error")
		return GBaseLogger::ERROR_LEVEL;
	else if(level == "fatal")
		return GBaseLogger::FATAL_LEVEL;
	return GBaseLogger::DEBUG_LEVEL;
}

