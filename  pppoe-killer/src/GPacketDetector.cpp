#ifndef WIN32
#include <arpa/inet.h>
#endif
#include <pcap.h>
#include "GPacketDetector.h"

using namespace glib;
using namespace std;

GPacketDetector::GPacketDetector(const std::string & expr, const string & name)
{
	pcap_if_t *alldevs, *d;
	char errbuf[PCAP_ERRBUF_SIZE];

	m_logger = GLogger::getLogger("packet");

	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		GERROR(*m_logger)("GPacketDetector pcap_findalldevs_ex=-1, %s", errbuf);
		return;
	}

	for(d = alldevs; d; d = d->next)
	{
		string s = d->name;
		if(s.find(name) != string::npos)
			break;
	}

	if(d == NULL)
	{
		GERROR(*m_logger)("GPacketDetector adapter %s not found", name.c_str());
		return;
	}

	_init(expr, d->name);
	pcap_freealldevs(alldevs);
}

GPacketDetector::~GPacketDetector()
{
	if(m_logger != NULL)
		delete m_logger;
}

void GPacketDetector::_init(const string & expr, const string & name)
{
	m_name = name;
	m_expression = expr;
	m_netmask = "255.255.255.0";
}

void GPacketDetector::AddReactor(const boost::signal2<void, const unsigned char *, int >::slot_type & slot)
{
	msig_detected.connect(slot);
}

void GPacketDetector::run()
{
	if(m_name == "" || m_expression == "")
	{
		return;
	}

	pcap_t *adhandle = NULL;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fcode;
	struct pcap_pkthdr *pkt_header;
	unsigned char *pkt_data;
	int res;
	
	string s = m_name;
	string::size_type n;

	if((n = s.find("://")) != string::npos)
		s = s.substr(n+3);

	//m_logger->error("%s", s.c_str());
	if ((adhandle = pcap_open_live(s.c_str(),							// name of the device
									 65536,								// portion of the packet to capture. 
																		// 65536 grants that the whole packet will be captured on all the MACs.
									 1,         // promiscuous mode
									 1000,								// read timeout
									 errbuf								// error buffer
									 )) == NULL)
	{
		GERROR(*m_logger)("Unable to open the adapter. %s is not supported by WinPcap\n", s.c_str());
		return;
	}
	
	//compile the filter
	if(pcap_compile(adhandle, &fcode, (char*)m_expression.c_str(), 1, inet_addr(m_netmask.c_str())) <0 ) {
		GERROR(*m_logger)("Unable to compile the packet filter. Check the syntax.");
		pcap_close(adhandle);
		return;
	}

	//set the filter
	if(pcap_setfilter(adhandle, &fcode) < 0) {
		GERROR(*m_logger)("Error setting the filter.");
		pcap_close(adhandle);
		return;
	}

	while(this->IsStopping() == false &&
			(res = pcap_next_ex( adhandle, &pkt_header, (const u_char**)&pkt_data)) >= 0) {
		if(res == 0) continue;

		//m_logger->info("packet detected, len: %d", pkt_header->len);
		GINFO(*m_logger)("packet detected, len: %d", pkt_header->len);
		msig_detected((const unsigned char*)pkt_data, pkt_header->len);
	}

	pcap_close(adhandle);
}

