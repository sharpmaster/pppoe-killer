#ifndef _HIPPOLIB_NETTOOL_H
#define _HIPPOLIB_NETTOOL_H

#include <vector>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#ifdef WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#else
#include <sys/socket.h>
#include <net/if.h>
#endif

namespace hippolib
{
	class netadapter
	{
	public:
		netadapter(const std::string & name) : m_name(name) {}
		netadapter(const netadapter & adapter)
		{
			m_name = adapter.m_name;
			m_description = adapter.m_description;
			m_mac = adapter.m_mac;
			m_addresses = adapter.m_addresses;
		}
		~netadapter() {};

		netadapter & operator=(const netadapter & adapter)
		{
			if(this != &adapter)
			{
				m_name = adapter.m_name;
				m_description = adapter.m_description;
				m_mac = adapter.m_mac;
				m_addresses = adapter.m_addresses;
			}

			return *this;
		}

		std::string getName()
		{
			return m_name;
		}

		void setName(const std::string & name)
		{
			m_name = name;
		}
		
		std::string getDescription()
		{
			return m_description;
		}
		
		void setDescription(const std::string & description)
		{
			m_description = description;
		}
		
		void
		addAddress(const boost::asio::ip::address_v4 & address,
						const boost::asio::ip::address_v4 & mask)
		{
			boost::array<boost::asio::ip::address_v4, 2> a;
			a[0] = address;
			a[1] = mask;
			m_addresses.push_back(a);
		}
		
		std::vector<boost::array<boost::asio::ip::address_v4, 2> >
		getAddresses()
		{
			return m_addresses;
		}
		
		boost::array<unsigned char, 6> getMac()
		{
			return m_mac;
		}
		
		void setMac(const boost::array<unsigned char, 6> & mac)
		{
			m_mac = mac;
		}
		
	private:
		std::string m_name;
		std::string m_description;
		boost::array<unsigned char, 6> m_mac;
		std::vector<boost::array<boost::asio::ip::address_v4, 2> > m_addresses;
	};
	
	class nettool
	{
	public:
		static boost::array<unsigned char, 6>
		parseMAC(const std::string & macstr)
		{
			boost::array<unsigned char, 6> ret;

			if(macstr.size() != 12)
				throw std::invalid_argument("Length Error");

			if(boost::algorithm::all(macstr, boost::algorithm::is_xdigit()) == false)
				throw std::invalid_argument("Format Error");

			for(int i = 0; i < 6; i++)
			{
				std::stringstream ss;
				short n;
				ss << std::hex << macstr[i*2] << macstr[i*2+1];
				ss >> n;
				ret[i] = (char)(n & 0xff);
			}

			return ret;
		}
		
		static std::string
		getMACString(const boost::array<unsigned char, 6> & macbin)
		{
			std::string ret = "";
			char buf[32];
			sprintf(buf, "%02X%02X%02X%02X%02X%02X",
				(unsigned char)macbin[0], (unsigned char)macbin[1],
				(unsigned char)macbin[2], (unsigned char)macbin[3],
				(unsigned char)macbin[4], (unsigned char)macbin[5]);
			ret = buf;
			return ret;
		}
		
		static bool isLocalMAC(const boost::array<unsigned char, 6> & macbin)
		{
			std::vector<hippolib::netadapter> adapters;

			adapters = hippolib::nettool::getLocalAdapters();
			BOOST_FOREACH(hippolib::netadapter a, adapters)
			{
				if(a.getMac() == macbin)
					return true;
			}
			return false;
		}
		
#ifdef WIN32
		static std::vector<hippolib::netadapter> getLocalAdapters()
		{
			PIP_ADAPTER_INFO pAdapterInfo;
			PIP_ADAPTER_INFO pAdapter = NULL;
			DWORD dwRetVal = 0;
			std::vector<hippolib::netadapter> ret;

			pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
			ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

			// Make an initial call to GetAdaptersInfo to get
			// the necessary size into the ulOutBufLen variable
			if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
			{
				free(pAdapterInfo);
				pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen); 
			}

			if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
			{
				pAdapter = pAdapterInfo;
				while (pAdapter)
				{
					if(pAdapter->Type != MIB_IF_TYPE_ETHERNET)
					{
						pAdapter = pAdapter->Next;
						continue;
					}
					
					hippolib::netadapter adapter(pAdapter->AdapterName);

					adapter.setDescription(pAdapter->Description);
					boost::array<unsigned char, 6> mac;
					for(int i = 0; i < 6; i++)
						mac[i] = *(pAdapter->Address+i);
					adapter.setMac(mac);

					IP_ADDR_STRING *pAddr = &(pAdapter->IpAddressList);
					while(pAddr)
					{
						boost::asio::ip::address_v4 addr = boost::asio::ip::address_v4::from_string(pAddr->IpAddress.String);
						boost::asio::ip::address_v4 mask = boost::asio::ip::address_v4::from_string(pAddr->IpMask.String);
						adapter.addAddress(addr, mask);
						pAddr = pAddr->Next;
					}

					ret.push_back(adapter);
					pAdapter = pAdapter->Next;
				}
			}

			free(pAdapterInfo);
			return ret;
		}
#else
		static std::vector<hippolib::netadapter> getLocalAdapters()
		{
			int sd;
			int num;
			char *pCardName;
			char buf[1024];
			
			struct sockaddr_in *localSock;
			unsigned long ip, mask;
			boost::array<unsigned char, 6> mac;
			
			struct ifconf ifc;
			short ifFlag;

			std::vector<hippolib::netadapter> ret;

			ifc.ifc_len = 1024;
			ifc.ifc_buf = buf;
			memset(buf, 0, 1024);
			sd = socket(AF_INET, SOCK_DGRAM, 0);

			if (ioctl(sd, SIOCGIFCONF, (char *)&ifc) < 0)
				return std::vector<hippolib::netadapter>();

			num = ifc.ifc_len / sizeof(struct ifreq);
			for (int i = 0; i < num; i++)
			{
				pCardName = ifc.ifc_ifcu.ifcu_req[i].ifr_name;
				if (pCardName == NULL)
					continue;

				if (ioctl(sd, SIOCGIFFLAGS, (char *)&(ifc.ifc_ifcu.ifcu_req[i])) < 0)
					continue;

				ifFlag = ifc.ifc_ifcu.ifcu_req[i].ifr_flags;
				if (!(ifFlag & IFF_UP) || (ifFlag & IFF_LOOPBACK))
					continue;
				
				localSock = (struct sockaddr_in *)&(ifc.ifc_ifcu.ifcu_req[i].ifr_addr);
				ip = ntohl(localSock->sin_addr.s_addr);
				if (ioctl(sd, SIOCGIFNETMASK, (char *)&(ifc.ifc_ifcu.ifcu_req[i])) < 0)
					continue;
				localSock = (struct sockaddr_in *)&(ifc.ifc_ifcu.ifcu_req[i].ifr_broadaddr);
				mask = ntohl(localSock->sin_addr.s_addr);

				if (ioctl(sd, SIOCGIFHWADDR, (char *)&(ifc.ifc_ifcu.ifcu_req[i])) < 0)
				{
					continue;
				}
				localSock = (struct sockaddr_in *)&(ifc.ifc_ifcu.ifcu_req[i].ifr_hwaddr);
				std::copy((unsigned char *)(ifc.ifc_ifcu.ifcu_req[i].ifr_hwaddr.sa_data),
					(unsigned char *)(ifc.ifc_ifcu.ifcu_req[i].ifr_hwaddr.sa_data) + 6,
					mac.begin());

				bool mac_found = false;
				BOOST_FOREACH(hippolib::netadapter & n, ret)
				{
					if(n.getMac() == mac)
					{
						n.addAddress(boost::asio::ip::address_v4(ip), boost::asio::ip::address_v4(mask));
						mac_found = true;
						break;
					}
				}

				if(mac_found == false)
				{
					std::string name(pCardName);
					std::string::size_type pos;

					if((pos = name.find_first_of(':')) != std::string::npos)
						name = name.substr(0, pos);

					hippolib::netadapter n(name);
					n.addAddress(boost::asio::ip::address_v4(ip), boost::asio::ip::address_v4(mask));
					n.setMac(mac);
					ret.push_back(n);
				}
			}
			
			return ret;
		}
#endif
		
		static boost::asio::ip::address_v4
		Resolve(const std::string & name)
		{
			try
			{
				boost::asio::ip::tcp::endpoint endpoint = ResolveTcpEndpoint(name, "");
				return endpoint.address().to_v4();
			}
			catch(...)
			{
				throw;
			}
		}

		static boost::asio::ip::tcp::endpoint
		ResolveTcpEndpoint(const std::string & name,
								const std::string & protocol)
		{
			boost::asio::io_service io_service;
			boost::asio::ip::tcp::resolver resolver(io_service);
			boost::asio::ip::tcp::resolver::query query(name, protocol);
			boost::asio::ip::tcp::resolver::iterator ite;
			boost::asio::ip::tcp::resolver::iterator end;

			try
			{
				ite = resolver.resolve(query);
			}
			catch(...)
			{
				throw;
			}
			if(ite == end)
				return boost::asio::ip::tcp::endpoint();

			return (*ite).endpoint();
		}
	};
}

#endif

