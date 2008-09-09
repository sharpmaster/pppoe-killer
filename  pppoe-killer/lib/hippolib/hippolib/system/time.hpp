#ifndef _HIPPOLIB_TIME_H
#define _HIPPOLIB_TIME_H

#include <string>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace hippolib
{
	class time
	{
	public:
		static std::string GetTimeString(const std::string & format,
										boost::posix_time::ptime time =
										boost::posix_time::second_clock::local_time())
		{
			using namespace boost::gregorian;
			using namespace boost::posix_time;
			
			/**
			* time_facet will be freed by locale
			* Delete it manually will cause serious problem!!
			*/
			time_facet *tf = new time_facet();
			std::stringstream ss;
			std::string ret;


			tf->format(format.c_str());
			std::locale loc(std::locale::classic(), tf);
			ss.imbue(loc);
			ss << time;
			ret.append(ss.str());

			return ret;
		}
	};
}

#endif

