#ifndef _GNETTOOL_H
#define _GNETTOOL_H
#include <glib/config.h>
#include <vector>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <glib/net/GNetAdapter.h>

namespace glib {
namespace net {
	
	class GLIB_EXPORT GNetTool
	{
	public:
		static boost::array<unsigned char, 6> parseMAC(const std::string & macstr);
		static std::string getMACString(const boost::array<unsigned char, 6> & macbin);
		static bool isLocalMAC(const boost::array<unsigned char, 6> & macbin);
		static std::vector<glib::net::GNetAdapter> getLocalAdapters();
		static boost::asio::ip::address_v4 Resolve(const std::string & name);
		static boost::asio::ip::tcp::endpoint ResolveTcpEndpoint(const std::string & name, const std::string & protocol);
	};
}}

#endif

