#include <stdexcept>
#include <boost/algorithm/string.hpp>
#include <glib/net/GHttpHeader.h>

using namespace std;
using namespace glib::net;

GHttpHeader::GHttpHeader() : m_name(""), m_value("")
{
}

GHttpHeader::GHttpHeader(const std::string & name, const std::string & value) : m_name(name), m_value(value)
{
}

GHttpHeader::GHttpHeader(const GHttpHeader & header) : m_name(header.m_name), m_value(header.m_value)
{
}

GHttpHeader& GHttpHeader::operator=(const GHttpHeader & header)
{
	if(this != &header)
	{
		m_name = header.m_name;
		m_value = header.m_value;
	}
	return *this;
}

string GHttpHeader::getName() const
{
	return m_name;
}

void GHttpHeader::setName(const string & name)
{
	m_name = name;
}

string GHttpHeader::getValue() const
{
	return m_value;
}

void GHttpHeader::setValue(const string & value)
{
	m_value = value;
}

string GHttpHeader::getString() const
{
	string ret = m_name + ": " + m_value;
	return ret;
}

GHttpHeader GHttpHeader::parseString(const string & str)
{
	string key, value;
	string::size_type index = str.find_first_of(':');

	if(index == string::npos)
		throw invalid_argument(str + " - No colon");
	if(index+1 >= str.size())
		throw invalid_argument(str + " - Format error");

	key = str.substr(0, index);
	if(index+2 < str.size())
		value = str.substr(index+2);
	else
		value = "";

	if(value.size() > 0 && value.at(value.size()-1) == '\r')
		value = value.substr(0, value.size()-1);

	return GHttpHeader(key, value);
}

bool HeaderKeyComparer::operator()(string s1, string s2) const
{
	return boost::algorithm::ilexicographical_compare(s1, s2);
}
