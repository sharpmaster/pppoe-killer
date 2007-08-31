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

