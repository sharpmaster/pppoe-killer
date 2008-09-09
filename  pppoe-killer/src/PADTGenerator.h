#ifndef _PADTGENERATOR_H
#define _PADTGENERATOR_H

#include "GPacketGenerator.h"
#include <hippolib/system/thread.hpp>

class PADTGenerator : public hippolib::thread, public GPacketGenerator
{
private:
	unsigned char m_srcmac[6];
	unsigned char m_dstmac[6];
	unsigned int m_interval;

public:
	PADTGenerator(const std::string & name, const unsigned char* src, const unsigned char* dst,
		unsigned int interval = 0);
	~PADTGenerator() {}
	unsigned int getInterval() {return m_interval;}
	void setInterval(unsigned int v) {m_interval = v;}
	
	void run();
};

#endif
