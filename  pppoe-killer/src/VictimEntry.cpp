#include "VictimEntry.h"

using namespace std;

VictimEntry::VictimEntry()
{
	m_ifname = "";
	m_lastseen = "";
	m_autokill = false;
	m_desc = "";
}

void VictimEntry::save(boost::archive::text_oarchive & ar, const unsigned int version) const
{
	for(int i = 0; i < 6; i++)
		ar & m_mac[i];
	ar & m_ifname;
	ar & m_lastseen;
	ar & m_autokill;
	ar & m_desc;
}

void VictimEntry::load(boost::archive::text_iarchive & ar, const unsigned int version)
{
	/**
	* Version 0 config
	*
	* 1. The MAC address
	* 2. The name of network interface where the MAC is detected
	* 3. The last time that is detected
	* 4. To kill automatically when startup (Not used)
	*/
	for(int i = 0; i < 6; i++)
		ar & m_mac[i];
	ar & m_ifname;
	ar & m_lastseen;
	ar & m_autokill;

	/**
	* Version 1 config
	*
	* 1. The user-defined description of the machine
	*/
	if(version > 0)
		ar & m_desc;
}

