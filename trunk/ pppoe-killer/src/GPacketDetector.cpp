#ifndef WIN32
#include <arpa/inet.h>
#endif
#include <pcap.h>
#include <log4cxx/mdc.h>
#include "GPacketDetector.h"

using namespace glib;
using namespace std;
using namespace log4cxx;

GPacketDetector::GPacketDetector(const std::string & expr, const string & name)
	: m_name(""), m_expression(expr), m_netmask("255.255.255.0")
{
	pcap_if_t *alldevs, *d;
	char errbuf[PCAP_ERRBUF_SIZE];

	m_logger = Logger::getLogger("packet");
	MDC::put("devname", name);
	MDC::put("expr", m_expression);

	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		LOG4CXX_ERROR(m_logger, "get devices error, " +  string(errbuf));
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
		LOG4CXX_ERROR(m_logger, "adapter " + name + " not found");
		pcap_freealldevs(alldevs);
		return;
	}

	m_name = d->name;
	pcap_freealldevs(alldevs);
}

GPacketDetector::~GPacketDetector()
{
	MDC::remove("devname");
	MDC::remove("expr");
}

void GPacketDetector::AddReactor(const boost::signal2<void, const unsigned char *, int >::slot_type & slot)
{
	msig_detected.connect(slot);
}

void GPacketDetector::run()
{
	if(m_name == "")
	{
		LOG4CXX_ERROR(m_logger, "packet detector not initialized"); 
		return;
	}

	LOG4CXX_DEBUG(m_logger, "detector starts");

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

	if ((adhandle = pcap_open_live(s.c_str(),							// name of the device
									 65536,								// portion of the packet to capture. 
																		// 65536 grants that the whole packet will be captured on all the MACs.
									 1,         // promiscuous mode
									 1000,								// read timeout
									 errbuf								// error buffer
									 )) == NULL)
	{
		LOG4CXX_ERROR(m_logger, "Unable to open the adapter. " + s + " is not supported by WinPcap");
		return;
	}
	
	//compile the filter
	if(pcap_compile(adhandle, &fcode, (char*)m_expression.c_str(), 1, inet_addr(m_netmask.c_str())) <0 )
	{
		LOG4CXX_ERROR(m_logger, "Unable to compile the packet filter. Check the syntax: " + m_expression);
		pcap_close(adhandle);
		return;
	}

	//set the filter
	if(pcap_setfilter(adhandle, &fcode) < 0)
	{
		LOG4CXX_ERROR(m_logger, "Error setting the filter");
		pcap_close(adhandle);
		return;
	}

	while(this->IsStopping() == false &&
			(res = pcap_next_ex( adhandle, &pkt_header, (const u_char**)&pkt_data)) >= 0)
	{
		if(res == 0) continue;
		msig_detected((const unsigned char*)pkt_data, pkt_header->len);
	}

	LOG4CXX_DEBUG(m_logger, "detector ends");
	pcap_close(adhandle);
}

