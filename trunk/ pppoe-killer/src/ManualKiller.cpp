#include <boost/scoped_ptr.hpp>
#include "PADTGenerator.h"
#include "ManualKiller.h"

ManualKiller::ManualKiller(const boost::array<unsigned char, 6> & src, const boost::array<unsigned char, 6> & dst,
		const std::string & name, const unsigned int interval)
		: Killer(KILLER_ID, src, dst, name, interval)
{
}

ManualKiller::~ManualKiller()
{
}

void ManualKiller::killthread()
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
	
	boost::scoped_ptr<PADTGenerator> padt_gnr(
			new PADTGenerator(getCardName(), getDstMAC().data(), getSrcMAC().data(), getPADTInterval()));

	padt_gnr->start();
	while(padt_gnr->isAlive())
	{
		if(IsStopping())
		{
			padt_gnr->waitStop();
			break;
		}
		GThread::sleep(100);
	}
}

