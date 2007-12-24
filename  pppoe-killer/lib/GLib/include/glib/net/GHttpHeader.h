#ifndef _GHTTPHEADER_H
#define _GHTTPHEADER_H
#include <glib/config.h>
#include <string>

namespace glib {
namespace net {
	class GLIB_EXPORT GHttpHeader
	{
	public:
		GHttpHeader();
		GHttpHeader(const std::string & name, const std::string & value);
		GHttpHeader(const GHttpHeader & header);
		GHttpHeader& operator=(const GHttpHeader & header);
		std::string getName() const;
		void setName(const std::string & name);
		std::string getValue() const;
		void setValue(const std::string & value);
		std::string getString() const;

		static GHttpHeader parseString(const std::string & str);
	private:
		std::string m_name;
		std::string m_value;
	};

	class GLIB_EXPORT HeaderKeyComparer
	{
	public:
		bool operator()(std::string s1, std::string s2) const;
	};
}}

#endif

