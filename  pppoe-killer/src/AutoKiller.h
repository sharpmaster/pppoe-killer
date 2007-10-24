#ifndef _AUTOKILLER_H
#define _AUTOKILLER_H

#include <glib/GThread.h>
#include <glib/GLogger.h>
#include <boost/scoped_ptr.hpp>
#include <boost/signal.hpp>
#include <boost/signals/slot.hpp>
#include <boost/thread/mutex.hpp>
#include "Killer.h"

class PADTGenerator;

class AutoKiller : public Killer
{
public:
	AutoKiller(const boost::array<unsigned char, 6> & src, const boost::array<unsigned char, 6> & dst,
		const std::string & name, const unsigned int interval = 0);
	~AutoKiller() {}
	
	static const int KILLER_ID = 0x1;
	
	void AddReactor(const boost::signal1<void, const unsigned char*>::slot_type& slot) {msig_detected.connect(slot);}
	
protected:
	void killthread();
	
private:
	boost::mutex m_padi_mutex;
	boost::scoped_ptr<PADTGenerator> m_padt_gnr;
	log4cxx::LoggerPtr m_logger;
	boost::signal1<void, const unsigned char*> msig_detected;

	void padi_detected(const unsigned char* packet, int len);
};

#endif

