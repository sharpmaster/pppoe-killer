#ifndef _MANUALKILLER_H
#define _MANUALKILLER_H

#include "Killer.h"

class ManualKiller : public Killer
{
public:
	ManualKiller(const boost::array<char, 6> & src, const boost::array<char, 6> & dst,
		const std::string & name, const unsigned int interval = 0);
	~ManualKiller();

	static const int KILLER_ID = 0x0;
protected:
	void killthread();
};

#endif

