#ifndef _PADTGENERATOR_H
#define _PADTGENERATOR_H

#include "GPacketGenerator.h"
#include <glib/GThread.h>

class PADTGenerator : public glib::GThread, public GPacketGenerator
{
private:
	unsigned char m_srcmac[6];
	unsigned char m_dstmac[6];

	glib::GBaseLogger *m_logger;
public:
	PADTGenerator(std::string & name, unsigned char* src, unsigned char* dst);
	PADTGenerator::~PADTGenerator();
	void run();
};

#endif
