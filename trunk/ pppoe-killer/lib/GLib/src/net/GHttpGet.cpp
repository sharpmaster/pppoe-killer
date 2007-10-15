#include <glib/net/GHttpGet.h>

using namespace std;
using namespace glib::net;

GHttpGet::GHttpGet() : GHttpMethod("GET")
{
}

GHttpGet::GHttpGet(const string & url) : GHttpMethod("GET")
{
	setUrl(url);
}

void GHttpGet::setUrl(const string & url)
{
	m_url = url;
	
	string::size_type index = m_url.find_first_of('/');
	if(index == string::npos)
	{
		setHost(m_url);
		setPath("/");
		m_querystring = "";
	}
	else
	{
		setHost(m_url.substr(0, index));
		string s = m_url.substr(index);
		string::size_type idx = s.find_first_of('?');
		if(idx == string::npos)
			setPath(s);
		else
		{
			setPath(s.substr(0, idx));
			m_querystring = s.substr(idx+1);
		}
	}	
}

string GHttpGet::getRequestLine()
{
	string ret = "";
	if(m_querystring.size() == 0)
		ret += ("GET " + getPath() + " HTTP/1.1");
	else
		ret += ("GET " + getPath() + "?" + m_querystring + " HTTP/1.1");
	return ret;
}

