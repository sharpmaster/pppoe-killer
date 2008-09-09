#ifndef _PACKETGENERATOR_H
#define _PACKETGENERATOR_H

#include <string>

class GPacketGenerator
{
private:
	void _initdevice(const std::string & name);
protected:
	std::string m_name;
	void* m_libc;
public:
	GPacketGenerator(const std::string & name);
	virtual ~GPacketGenerator();

	void Clear();
	virtual bool Send();
};

#endif
