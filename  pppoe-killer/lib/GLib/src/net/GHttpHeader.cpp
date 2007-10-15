#include <glib/net/GHttpHeader.h>
#include <stdexcept>

using namespace std;
using namespace glib::net;

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
	string::size_type index = str.find_first_of(':');

	if(index == string::npos)
		throw invalid_argument("No colon");
	if(index+2 >= str.size())
		throw invalid_argument("Format error");

	string key = str.substr(0, index);
	string value = str.substr(index+2);

	if(value.at(value.size()-1) == '\r')
		value = value.substr(0, value.size()-1);

	return GHttpHeader(key, value);
}

