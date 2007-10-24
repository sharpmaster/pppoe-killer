#ifndef _PADTGENERATOR_H
#define _PADTGENERATOR_H

#include "GPacketGenerator.h"
#include <glib/GThread.h>

class PADTGenerator : public glib::GThread, public GPacketGenerator
{
private:
	unsigned char m_srcmac[6];
	unsigned char m_dstmac[6];
	unsigned int m_interval;

	log4cxx::LoggerPtr m_logger;
public:
	PADTGenerator(const std::string & name, const unsigned char* src, const unsigned char* dst,
		unsigned int interval = 0);
	~PADTGenerator();
	unsigned int getInterval() {return m_interval;}
	void setInterval(unsigned int v) {m_interval = v;}
	
	void run();
};

#endif
