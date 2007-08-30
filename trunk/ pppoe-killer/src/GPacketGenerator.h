#ifndef _PACKETGENERATOR_H
#define _PACKETGENERATOR_H

#include <glib/GLogger.h>
#include <string>

class GPacketGenerator
{
private:
	glib::GBaseLogger *m_logger;
	void _initdevice(const std::string & name);
protected:
	std::string m_name;
	void* m_libc;
public:
	GPacketGenerator(const std::string & name);
	~GPacketGenerator();

	void Clear();
	virtual bool Send();
};

#endif