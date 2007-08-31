#ifndef _VICTIMENTRY_H
#define _VICTIMENTRY_H

#include <string>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/noncopyable.hpp>
#include <glib/GLogger.h>
#include "AutoKiller.h"

class VictimEntry : boost::noncopyable
{
public:
	VictimEntry();
		
	void setMac(const boost::array<char, 6> & mac) {m_mac = mac;}
	boost::array<char, 6> getMac() const {return m_mac;}
	void setInterfaceName(const std::string & name) {m_ifname = name;}
	std::string getInterfaceName() const {return m_ifname;}
	void setLastSeenDate(const std::string & date) {m_lastseen = date;}
	std::string getLastSeenDate() const {return m_lastseen;}
	void setAutoKill(bool autokill) {m_autokill = autokill;}
	bool getAutoKill() {return m_autokill;}
	
	void setAutoKiller(const boost::shared_ptr<AutoKiller> killer) {m_killer = killer;}
	void startAutoKiller() {if(m_killer)m_killer->start();}
	void stopAutoKiller() {if(m_killer)m_killer->stop();}
	bool isAutoKillerAlive() {if(m_killer)return m_killer->isAlive();else return false;}
	bool isAutoKillerExisted() {return !(m_killer.get() == 0);}
private:
	boost::array<char, 6> m_mac;
	std::string m_ifname;
	std::string m_lastseen;
	bool m_autokill;
	boost::shared_ptr<AutoKiller> m_killer;

	friend class boost::serialization::access;
	void save(boost::archive::text_oarchive & ar, const unsigned int version) const;
	void load(boost::archive::text_iarchive & ar, const unsigned int version);
	BOOST_SERIALIZATION_SPLIT_MEMBER()
};

#endif