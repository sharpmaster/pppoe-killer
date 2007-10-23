#include "PADTGenerator.h"
#include <libnet.h>

using namespace glib;

PADTGenerator::PADTGenerator(const std::string & name, const unsigned char *src, const unsigned char *dst,
						unsigned int interval) : GPacketGenerator(name), m_interval(interval)
{
	memcpy(m_srcmac, src, 6);
	memcpy(m_dstmac, dst, 6);

	m_logger = GLogger::getLogger("packet");
}

PADTGenerator::~PADTGenerator()
{
	delete m_logger;
}

void PADTGenerator::run()
{
	unsigned short session, tmp;
	unsigned char payload[6];
	libnet_ptag_t ethernet = 0;
	int ret;

	GDEBUG(*m_logger)("PADTGenerator::run() starts");

	if(m_libc == NULL)
	{
		GERROR(*m_logger)("PADTGenerator libc NULL");
		return;
	}

	payload[0] = 0x11;
	payload[1] = 0xa7;
	payload[4] = 0x0;
	payload[5] = 0x0;
	session = 0;
	for(int i = 0; i <= 65535; i++)
	{
		if(IsStopping())
			break;
		
		tmp = htons(session++);
		memcpy(&payload[2], &tmp, 2);

		libnet_clear_packet((libnet_t*)m_libc);
		ethernet = libnet_build_ethernet(
						m_dstmac,
						m_srcmac,
						0x8863,
						(u_int8_t*)payload,
						(u_int32_t)6,
						(libnet_t*)m_libc,
						0);
		if(ethernet == -1)
		{
			GERROR(*m_logger)("PADTGenerator ethernet==-1");
			continue;
		}

		ret = libnet_write((libnet_t*)m_libc);
		if(ret == -1)
		{
			GERROR(*m_logger)("PADTGenerator ret==-1");
			continue;
		}

		if(m_interval != 0)
			GThread::sleep(m_interval);
	}

	GDEBUG(*m_logger)("PADTGenerator::run() ends");
}
