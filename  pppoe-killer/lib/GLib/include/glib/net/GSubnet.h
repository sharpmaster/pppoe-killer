#ifndef _GSUBNET_H
#define _GSUBNET_H
#include <glib/config.h>
#include <boost/asio.hpp>
#include <boost/regex.hpp>

namespace glib {
namespace net {

	class GLIB_EXPORT GSubnet
	{
	public:
		GSubnet() {}
		GSubnet(const boost::asio::ip::address_v4 & network, const boost::asio::ip::address_v4 & mask);
		GSubnet(const GSubnet & subnet);
		GSubnet & operator=(const GSubnet & subnet);

		bool containsIp(const boost::asio::ip::address_v4 & ip);
		static GSubnet buildSubnet(const boost::asio::ip::address_v4 & network, const boost::asio::ip::address_v4 & mask);
		static GSubnet parseSubnet(const std::string & str);
		
		void setNetwork(const boost::asio::ip::address_v4 & network) {m_network = network;}
		boost::asio::ip::address_v4 getNetwork() {return m_network;}
		void setMask(const boost::asio::ip::address_v4 & mask) {m_mask = mask;}
		boost::asio::ip::address_v4 getMask() {return m_mask;}
	private:
		boost::asio::ip::address_v4 m_network;
		boost::asio::ip::address_v4 m_mask;

		static const boost::regex EXPR_SINGLE;
		static const boost::regex EXPR_SUBNET;
		static const boost::regex EXPR_SIMPLESUBNET;
	};
}}
#endif
