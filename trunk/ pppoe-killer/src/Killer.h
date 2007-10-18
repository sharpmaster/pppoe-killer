#ifndef _KILLER_H
#define _KILLER_H

#include <boost/array.hpp>
#include <glib/GThread.h>

class Killer : public glib::GThread
{
public:
	Killer(const int id,
			const boost::array<char, 6> & src,
			const boost::array<char, 6> & dst,
			const std::string & card, const unsigned int interval = 0)
			: m_id(id), m_srcmac(src), m_dstmac(dst), m_card(card), m_interval(interval) {}
	
	virtual ~Killer() {}

	int getID() const {return m_id;}
	void setSrcMAC(const boost::array<char, 6> & src) {m_srcmac = src;}
	boost::array<char, 6> getSrcMAC() {return m_srcmac;}
	void setDstMAC(const boost::array<char, 6> & dst) {m_dstmac = dst;}
	boost::array<char, 6> getDstMAC() {return m_dstmac;}
	void setCardName(const std::string & card) {m_card = card;}
	std::string getCardName() {return m_card;}
	void setPADTInterval(unsigned int v) {m_interval = v;}
	unsigned int getPADTInterval() const {return m_interval;}

	void run() {killthread();}
protected:
	virtual void killthread() = 0;
private:
	int m_id;
	boost::array<char, 6> m_srcmac;
	boost::array<char, 6> m_dstmac;
	std::string m_card;
	unsigned int m_interval;
};

#endif

