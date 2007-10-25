#ifndef _GPACKETDETECTOR_H
#define _GPACKETDETECTOR_H

#include <string>
#include <boost/signal.hpp>
#include <boost/signals/slot.hpp>
#include <log4cxx/logger.h>
#include <glib/GThread.h>
#include <glib/GLogger.h>

class GPacketDetector : public glib::GThread {
public:
	GPacketDetector(const std::string & expr, const std::string & name);
	~GPacketDetector();

	bool isInitialized() {return !(m_name == "");}
	void AddReactor(const boost::signal2<void, const unsigned char*, int>::slot_type& slot);
	void run();
private:
	std::string m_name;
	std::string m_expression;
	/**
	* Netmask parameter only works under "ip broadcast" filtering
	* In other conditions it has no meanings
	*/
	std::string m_netmask;

	boost::signal2<void, const unsigned char*, int> msig_detected;
	log4cxx::LoggerPtr m_logger;
};

#endif

