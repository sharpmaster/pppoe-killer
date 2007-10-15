#ifndef _GPROPERTIES_H
#define _GPROPERTIES_H
#include <glib/config.h>
#include <map>
#include <string>
#include <istream>
#include <ostream>

namespace glib
{
	class GLIB_EXPORT GProperties : public std::map<std::string, std::string>
	{
	public:
		GProperties() {}
		~GProperties() {}
		bool load(std::istream & istr);
		bool save(std::ostream & ostr);
		std::string get(const std::string & key, const std::string & defaultv = "") const;
		void set(const std::string & key, const std::string & value);
	};
}

#endif

