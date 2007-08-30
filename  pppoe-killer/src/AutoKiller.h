#ifndef _AUTOKILLER_H
#define _AUTOKILLER_H

#include <glib/GThread.h>
#include <glib/GLogger.h>
#include <boost/signal.hpp>
#include <boost/signals/slot.hpp>

class AutoKiller : public glib::GThread
{
private:
	unsigned char m_srcmac[6];
	unsigned char m_dstmac[6];
	std::string m_card;
	glib::GBaseLogger *m_logger;
	boost::signal1<void, const unsigned char*> msig_detected;

	void padi_detected(const unsigned char* packet, int len);
public:
	AutoKiller(unsigned char *src, unsigned char *dst, std::string & name);
	~AutoKiller();

	void AddReactor(const boost::signal1<void, const unsigned char*>::slot_type& slot) {msig_detected.connect(slot);}
	void run();
};

#endif
