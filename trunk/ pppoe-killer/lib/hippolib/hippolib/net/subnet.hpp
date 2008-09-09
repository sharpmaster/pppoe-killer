#ifndef _HIPPOLIB_SUBNET_H
#define _HIPPOLIB_SUBNET_H

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

namespace hippolib
{

	class subnet
	{
	public:
		subnet() {}
		subnet(const boost::asio::ip::address_v4 & network,
				const boost::asio::ip::address_v4 & mask)
				: m_network(network), m_mask(mask)
		{
		}
		
		subnet(const subnet & net)
		{
			m_network = net.m_network;
			m_mask = net.m_mask;
		}
		
		subnet & operator=(const subnet & net)
		{
			if(this != &net)
			{
				m_network = net.m_network;
				m_mask = net.m_mask;
			}

			return *this;
		}

		bool containsIp(const boost::asio::ip::address_v4 & ip)
		{
			unsigned long ipbit = ip.to_ulong();
			unsigned long netbit = m_network.to_ulong();
			unsigned long maskbit = m_mask.to_ulong();

			if((ipbit & maskbit) == netbit)
				return true;

			return false;
		}
		
		static subnet
		buildSubnet(const boost::asio::ip::address_v4 & network,
						const boost::asio::ip::address_v4 & mask)
		{
			unsigned long maskbit = mask.to_ulong();
			bool bitend = false;
			for(int i = 0; i < 32; i++)
			{
				if((maskbit & (1 << i)) > 0)
					bitend = true;
				else
				{
					if(bitend == true)
						throw std::invalid_argument("mask error");
				}
			}

			return subnet(network, mask);
		}
		
		static subnet parseSubnet(const std::string & str)
		{
			static boost::regex EXPR_SINGLE = boost::regex("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
			static boost::regex EXPR_SUBNET = boost::regex("([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})/([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})");
			static boost::regex EXPR_SIMPLESUBNET = boost::regex("([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})/([0-9]+)");
			boost::cmatch what;

			if(regex_match(str.c_str(), what, EXPR_SINGLE))
			{
				boost::system::error_code e;
				boost::asio::ip::address_v4 ip = boost::asio::ip::address_v4::from_string(str, e);
				if(e)
					throw std::invalid_argument(e.message());
				boost::asio::ip::address_v4 mask = boost::asio::ip::address_v4::from_string("255.255.255.255");
				return subnet(ip, mask);
			}
			else if(regex_match(str.c_str(), what, EXPR_SUBNET))
			{
				std::string netidstr, maskstr;
				boost::system::error_code e;
				
				netidstr.assign(what[1].first, what[1].second);
				maskstr.assign(what[2].first, what[2].second);

				boost::asio::ip::address_v4 netid = boost::asio::ip::address_v4::from_string(netidstr, e);
				if(e)
					throw std::invalid_argument(e.message());
				boost::asio::ip::address_v4 mask = boost::asio::ip::address_v4::from_string(maskstr, e);
				if(e)
					throw std::invalid_argument(e.message());

				subnet net;
				try
				{
					net = buildSubnet(netid, mask);
				}
				catch(...)
				{
					throw;
				}

				return net;
			}
			else if(regex_match(str.c_str(), what, EXPR_SIMPLESUBNET))
			{
				std::string netidstr, maskstr;
				boost::system::error_code e;

				netidstr.assign(what[1].first, what[1].second);
				maskstr.assign(what[2].first, what[2].second);

				boost::asio::ip::address_v4 netid = boost::asio::ip::address_v4::from_string(netidstr, e);
				if(e)
					throw std::invalid_argument(e.message());
				unsigned int maskbit;
				try
				{
					maskbit = boost::lexical_cast<unsigned int>(maskstr);
				}
				catch(...)
				{
					throw;
				}

				if(maskbit > 32)
					throw std::invalid_argument("mask error");

				unsigned long masklong = 0;
				for(unsigned int i = 0; i < maskbit; i++)
					masklong |= (1 << (31 - i));

				boost::asio::ip::address_v4 mask = boost::asio::ip::address_v4(masklong);
				subnet net;
				try
				{
					net = buildSubnet(netid, mask);
				}
				catch(...)
				{
					throw;
				}

				return net;
			}
			else
			{
				throw std::invalid_argument("format error");
			}
			
			return subnet();
		}
		
		void setNetwork(const boost::asio::ip::address_v4 & network)
		{
			m_network = network;
		}
		
		boost::asio::ip::address_v4 getNetwork()
		{
			return m_network;
		}
		
		void setMask(const boost::asio::ip::address_v4 & mask)
		{
			m_mask = mask;
		}
		
		boost::asio::ip::address_v4 getMask()
		{
			return m_mask;
		}
		
	private:
		boost::asio::ip::address_v4 m_network;
		boost::asio::ip::address_v4 m_mask;
	};
}

#endif

