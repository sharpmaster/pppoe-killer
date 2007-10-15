#include <glib/GProperties.h>

using namespace std;
using namespace glib;

bool GProperties::load(istream & istr)
{
	char buf[1024];
	string bufstr, left, right;
	string::size_type idx;

	clear();
	while(!istr.eof())
	{
		istr.getline(buf, 1024);
		if(buf[0] == '\0' || buf[0] == '#')
			continue;

		bufstr = buf;
		if((idx = bufstr.find_first_of('=')) == string::npos)
			continue;
		left = bufstr.substr(0, idx);
		if(idx < bufstr.size()-1)
			right = bufstr.substr(idx+1);
		else
			right = "";
		insert(make_pair(left, right));
	}

	return true;
}

bool GProperties::save(ostream & ostr)
{
	string buf;
	map<string, string>::iterator i;

	i = begin();
	while(i != end())
	{
		buf = i->first + "=" + i->second;
		ostr.write(buf.c_str(), (streamsize)buf.size());
		ostr.put('\n');
		i++;
	}

	return true;
}

string GProperties::get(const string & key, const string & defaultv) const
{
	map<string, string>::const_iterator i;

	i = find(key);
	if(i == end())
		return defaultv;

	return i->second;
}

void GProperties::set(const string & key, const string & value)
{
	map<string, string>::iterator i;

	i = find(key);
	if(i == end())
		insert(make_pair(key, value));
	else
		i->second = value;
}

