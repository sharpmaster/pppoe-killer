#ifndef _GTIME_H
#define _GTIME_H
#include <glib/config.h>
#include <string>

namespace glib
{
	class GLIB_EXPORT GTime
	{
	public:
		static std::string GetTimeString();
		static std::string GetHTTPTimeString();
	};
}

#endif

