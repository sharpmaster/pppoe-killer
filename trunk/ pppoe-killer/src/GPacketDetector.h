#ifndef _GPACKETDETECTOR_H
#define _GPACKETDETECTOR_H

#include <string>
#include <boost/signal.hpp>
#include <boost/signals/slot.hpp>
#include <glib/GThread.h>
#include <glib/GLogger.h>

class GPacketDetector : public glib::GThread {
private:
	std::string m_name;
	std::string m_expression;
	/**
	* Netmask parameter only works under "ip broadcast" filtering
	* In other conditions it has no meanings
	*/
	std::string m_netmask;

	boost::signal2<void, const unsigned char*, int> msig_detected;
	glib::GBaseLogger *m_logger;

	void _init(const std::string & expr, const std::string & name);
public:
	GPacketDetector(const std::string & expr, const std::string & name);
	virtual ~GPacketDetector();

	void AddReactor(const boost::signal2<void, const unsigned char*, int>::slot_type& slot);
	void run();
};

#endif

