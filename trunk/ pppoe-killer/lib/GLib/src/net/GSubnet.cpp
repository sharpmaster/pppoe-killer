#include <glib/net/GSubnet.h>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace glib::net;
using namespace boost::asio::ip;

GSubnet::GSubnet(const address_v4 & network, const address_v4 & mask) : m_network(network), m_mask(mask)
{
}

GSubnet::GSubnet(const GSubnet & subnet)
{
	m_network = subnet.m_network;
	m_mask = subnet.m_mask;
}

GSubnet & GSubnet::operator=(const GSubnet & subnet)
{
	if(this != &subnet)
	{
		m_network = subnet.m_network;
		m_mask = subnet.m_mask;
	}

	return *this;
}

bool GSubnet::containsIp(const address_v4 & ip)
{
	unsigned long ipbit = ip.to_ulong();
	unsigned long netbit = m_network.to_ulong();
	unsigned long maskbit = m_mask.to_ulong();

	if((ipbit & maskbit) == netbit)
		return true;

	return false;
}

GSubnet GSubnet::buildSubnet(const address_v4 & network, const address_v4 & mask)
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
				throw invalid_argument("mask error");
		}
	}

	return GSubnet(network, mask);
}

const boost::regex GSubnet::EXPR_SINGLE = boost::regex("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
const boost::regex GSubnet::EXPR_SUBNET = boost::regex("([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})/([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})");
const boost::regex GSubnet::EXPR_SIMPLESUBNET = boost::regex("([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})/([0-9]+)");

GSubnet GSubnet::parseSubnet(const string & str)
{
	boost::cmatch what;

	if(regex_match(str.c_str(), what, EXPR_SINGLE))
	{
		boost::asio::error e;
		address_v4 ip = address_v4::from_string(str, boost::asio::assign_error(e));
		if(e)
			throw invalid_argument(e.what());
		address_v4 mask = address_v4::from_string("255.255.255.255");
		return GSubnet(ip, mask);
	}
	else if(regex_match(str.c_str(), what, EXPR_SUBNET))
	{
		string netidstr, maskstr;
		boost::asio::error e;
		
		netidstr.assign(what[1].first, what[1].second);
		maskstr.assign(what[2].first, what[2].second);

		address_v4 netid = address_v4::from_string(netidstr, boost::asio::assign_error(e));
		if(e)
			throw invalid_argument(e.what());
		address_v4 mask = address_v4::from_string(maskstr, boost::asio::assign_error(e));
		if(e)
			throw invalid_argument(e.what());

		GSubnet net;
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
		string netidstr, maskstr;
		boost::asio::error e;

		netidstr.assign(what[1].first, what[1].second);
		maskstr.assign(what[2].first, what[2].second);

		address_v4 netid = address_v4::from_string(netidstr, boost::asio::assign_error(e));
		if(e)
			throw invalid_argument(e.what());
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
			throw invalid_argument("mask error");

		unsigned long masklong = 0;
		for(unsigned int i = 0; i < maskbit; i++)
			masklong |= (1 << (31 - i));

		address_v4 mask = address_v4(masklong);
		GSubnet net;
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
		throw invalid_argument("format error");
	}
	
	return GSubnet();
}
