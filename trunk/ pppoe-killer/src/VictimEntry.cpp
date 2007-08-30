#include <glib/GBase64.h>
#include "VictimEntry.h"

using namespace std;
using namespace glib;

VictimEntry::VictimEntry()
{
	m_ifname = "";
	m_lastseen = "";
	m_autokill = false;
}

void VictimEntry::save(boost::archive::text_oarchive & ar, const unsigned int version) const
{
	for(int i = 0; i < 6; i++)
		ar & m_mac[i];
	ar & m_ifname;
	ar & m_lastseen;
	ar & m_autokill;
}

void VictimEntry::load(boost::archive::text_iarchive & ar, const unsigned int version)
{
	for(int i = 0; i < 6; i++)
		ar & m_mac[i];
	ar & m_ifname;
	ar & m_lastseen;
	ar & m_autokill;
}

/*
string VictimEntry::GetXMLElementStr()
{
	string ret = "<victim";

	string s;
	s.assign(m_mac.data(), 6);
	s = GBase64::Encode(s);
	ret += " mac=\"";
	ret += s;
	ret += "\"";
		
	ret += " ifname=\"";
	ret += m_ifname;
	ret += "\"";

	ret += " lastseen=\"";
	ret += m_lastseen;
	ret += "\"";

	ret += " autokill=\"";
	if(m_autokill)
		ret += "1\"";
	else
		ret += "0\"";

	ret += "/>";

	return ret;
}

bool VictimEntry::SetXMLElement(apr_xml_elem *e)
{
	string str;

	str = aprXmlGetPropString(e, "mac", "");
	if(str == "")
		return false;
	str = GBase64::Decode(str);
	if(str.size() != 6)
		return false;
	copy(str.begin(), str.end(), m_mac.begin());
	

	str = aprXmlGetPropString(e, "ifname", "");
	if(str == "")
		return false;
	m_ifname = str;

	str = aprXmlGetPropString(e, "lastseen", "");
	if(str == "")
		return false;
	m_lastseen = str;

	m_autokill = aprXmlGetPropBool(e, "autokill", false);

	return true;
}
*/

