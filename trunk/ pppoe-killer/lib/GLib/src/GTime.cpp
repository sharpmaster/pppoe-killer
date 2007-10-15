#include <glib/GTime.h>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace glib;

std::string GTime::GetTimeString()
{
	using namespace boost::gregorian;
	using namespace boost::posix_time;
	
	ptime t(second_clock::local_time());
	/**
	* time_facet will be freed by locale
	* Delete it manually will cause serious problem!!
	*/
	time_facet *tf = new time_facet();
	std::stringstream ss;
	std::string ret;


	tf->format("%Y-%m-%d %H:%M:%S");
	std::locale loc(std::locale::classic(), tf);
	ss.imbue(loc);
	ss << t;
	ret.append(ss.str());

	return ret;
}

std::string GTime::GetHTTPTimeString()
{
	using namespace boost::gregorian;
	using namespace boost::posix_time;
	
	ptime t(second_clock::local_time());
	/**
	* time_facet will be freed by locale
	* Delete it manually will cause serious problem!!
	*/
	time_facet *tf = new time_facet();
	std::stringstream ss;
	std::string ret;


	tf->format("%a, %d %b %Y %H:%M:%S");
	std::locale loc(std::locale::classic(), tf);
	ss.imbue(loc);
	ss << t;
	ret.append(ss.str());

	return ret;
}

