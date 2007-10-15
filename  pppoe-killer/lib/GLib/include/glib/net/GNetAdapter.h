#ifndef _GNETADAPTER_H
#define _GNETADAPTER_H
#include <glib/config.h>
#include <vector>
#include <boost/array.hpp>
#include <boost/asio.hpp>

namespace glib {
namespace net {
	
	class GLIB_EXPORT GNetAdapter
	{
	public:
		GNetAdapter(const std::string & name);
		GNetAdapter(const GNetAdapter & adapter);
		~GNetAdapter() {};

		GNetAdapter & operator=(const GNetAdapter & adapter);

		std::string getName() {return m_name;}
		void setName(const std::string & name) {m_name = name;}
		std::string getDescription() {return m_description;}
		void setDescription(const std::string & description) {m_description = description;}
		void addAddress(const boost::asio::ip::address_v4 & address, const boost::asio::ip::address_v4 & mask);
		std::vector<boost::array<boost::asio::ip::address_v4, 2> > getAddresses() {return m_addresses;}
		boost::array<char, 6> getMac() {return m_mac;}
		void setMac(const boost::array<char, 6> & mac) {m_mac = mac;}
	private:
		std::string m_name;
		std::string m_description;
		boost::array<char, 6> m_mac;
		std::vector<boost::array<boost::asio::ip::address_v4, 2> > m_addresses;
	};
}}

#endif

