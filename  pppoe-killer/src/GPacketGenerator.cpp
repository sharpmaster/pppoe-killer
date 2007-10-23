#include <pcap.h>
#include <libnet.h>
#include "GPacketGenerator.h"

using namespace glib;
using namespace std;

GPacketGenerator::GPacketGenerator(const string & name)
{
	pcap_if_t *alldevs, *d;
	char errbuf[PCAP_ERRBUF_SIZE];

	m_libc = NULL;
	m_logger = GLogger::getLogger("packet");

	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		GERROR(*m_logger)("GPacketGenerator pcap_findalldevs_ex=-1, %s", errbuf);
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
		GERROR(*m_logger)("GPacketGenerator no adapter %s not found", name.c_str());
		return;
	}

	_initdevice(d->name);
	pcap_freealldevs(alldevs);
}

GPacketGenerator::~GPacketGenerator()
{
	delete m_logger;
	if(m_libc != NULL)
		libnet_destroy((libnet_t*)m_libc);
}

void GPacketGenerator::_initdevice(const string & name) {
	char errbuf[PCAP_ERRBUF_SIZE];
	string s;
	string::size_type n;

	s = name;

	if((n = s.find("://")) != string::npos)
		s = s.substr(n+3);

	m_name = s;
	m_libc = (void*)libnet_init(LIBNET_LINK, (char*)m_name.c_str(), errbuf);
	if(m_libc == NULL)
		GERROR(*m_logger)("GPacketGenerator lib=NULL name: %s, %s", (char*)m_name.c_str(), errbuf);
}

void GPacketGenerator::Clear()
{
	libnet_clear_packet((libnet_t*)m_libc);
}

bool GPacketGenerator::Send()
{
	int ret;
	ret = libnet_write((libnet_t*)m_libc);
	if(ret == -1)
		return false;

	return true;
}

