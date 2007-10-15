#include <glib/net/GNetTool.h>
#ifdef WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#endif
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace std;
using namespace glib::net;
using namespace boost::asio::ip;
namespace ba = boost::algorithm;

boost::array<char, 6> GNetTool::parseMAC(const string & macstr)
{
	boost::array<char, 6> ret;

	if(macstr.size() != 12)
		throw invalid_argument("Length Error");

	if(ba::all(macstr, ba::is_xdigit()) == false)
		throw invalid_argument("Format Error");

	for(int i = 0; i < 6; i++)
	{
		stringstream ss;
		short n;
		ss << hex << macstr[i*2] << macstr[i*2+1];
		ss >> n;
		ret[i] = (char)(n & 0xff);
	}

	return ret;
}

string GNetTool::getMACString(const boost::array<char, 6> & macbin)
{
	string ret = "";
	char buf[32];
	sprintf(buf, "%02X%02X%02X%02X%02X%02X",
		(unsigned char)macbin[0], (unsigned char)macbin[1],
		(unsigned char)macbin[2], (unsigned char)macbin[3],
		(unsigned char)macbin[4], (unsigned char)macbin[5]);
	ret = buf;
	return ret;
}

bool GNetTool::isLocalMAC(const boost::array<char, 6> & macbin)
{
	vector<GNetAdapter> adapters;

	adapters = GNetTool::getLocalAdapters();
	BOOST_FOREACH(GNetAdapter a, adapters)
	{
		if(a.getMac() == macbin)
			return true;
	}
	return false;
}

#ifdef WIN32
vector<GNetAdapter> GNetTool::getLocalAdapters()
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	vector<GNetAdapter> ret;

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
			
			GNetAdapter adapter(pAdapter->AdapterName);

			adapter.setDescription(pAdapter->Description);
			boost::array<char, 6> mac;
			for(int i = 0; i < 6; i++)
				mac[i] = *(pAdapter->Address+i);
			adapter.setMac(mac);

			IP_ADDR_STRING *pAddr = &(pAdapter->IpAddressList);
			while(pAddr)
			{
				address_v4 addr = address_v4::from_string(pAddr->IpAddress.String);
				address_v4 mask = address_v4::from_string(pAddr->IpMask.String);
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
vector<GNetAdapter> GNetTool::getLocalAdapters()
{
	return vector<GNetAdapter>();
}
#endif

address_v4 GNetTool::Resolve(const string & name)
{
	try
	{
		tcp::endpoint endpoint = ResolveTcpEndpoint(name, "");
		return endpoint.address().to_v4();
	}
	catch(...)
	{
		throw;
	}
}

boost::asio::ip::tcp::endpoint GNetTool::ResolveTcpEndpoint(const std::string & name, const std::string & protocol)
{
	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(name, protocol);
	tcp::resolver::iterator ite;
	tcp::resolver::iterator end;

	ite = resolver.resolve(query, boost::asio::throw_error());
	if(ite == end)
		return tcp::endpoint();

	return (*ite).endpoint();
}
