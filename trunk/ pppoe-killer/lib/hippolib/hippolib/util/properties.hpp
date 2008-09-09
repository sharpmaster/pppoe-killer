#ifndef _HIPPOLIB_PROPERTIES_H
#define _HIPPOLIB_PROPERTIES_H

#include <map>
#include <string>
#include <istream>
#include <ostream>

namespace hippolib
{
	class properties : public std::map<std::string, std::string>
	{
	public:
		bool load(std::istream & istr)
		{
			char buf[1024];
			std::string bufstr, left, right;
			std::string::size_type idx;

			clear();
			while(!istr.eof())
			{
				istr.getline(buf, 1024);
				if(buf[0] == '\0' || buf[0] == '#')
					continue;

				bufstr = buf;
				if((idx = bufstr.find_first_of('=')) == std::string::npos)
					continue;
				left = bufstr.substr(0, idx);
				if(idx < bufstr.size()-1)
					right = bufstr.substr(idx+1);
				else
					right = "";
				insert(std::make_pair(left, right));
			}

			return true;
		}
		
		bool save(std::ostream & ostr)
		{
			std::string buf;
			std::map<std::string, std::string>::const_iterator i;

			i = begin();
			while(i != end())
			{
				buf = i->first + "=" + i->second;
				ostr.write(buf.c_str(), (std::streamsize)buf.size());
				ostr.put('\n');
				i++;
			}

			return true;
		}
		
		std::string get(const std::string & key, const std::string & defaultv = "") const
		{
			std::map<std::string, std::string>::const_iterator i;

			i = find(key);
			if(i == end())
				return defaultv;

			return i->second;
		}
		
		void set(const std::string & key, const std::string & value)
		{
			std::map<std::string, std::string>::iterator i;

			i = find(key);
			if(i == end())
				insert(std::make_pair(key, value));
			else
				i->second = value;
		}
	};
}

#endif

