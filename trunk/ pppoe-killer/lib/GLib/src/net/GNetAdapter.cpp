#include <glib/net/GNetAdapter.h>

using namespace std;
using namespace glib::net;
using namespace boost::asio::ip;

GNetAdapter::GNetAdapter(const string & name) : m_name(name)
{
}

GNetAdapter::GNetAdapter(const GNetAdapter & adapter)
{
	m_name = adapter.m_name;
	m_description = adapter.m_description;
	m_mac = adapter.m_mac;
	m_addresses = adapter.m_addresses;
}

GNetAdapter & GNetAdapter::operator=(const GNetAdapter & adapter)
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

void GNetAdapter::addAddress(const address_v4 & address, const address_v4 & mask)
{
	boost::array<address_v4, 2> a;
	a[0] = address;
	a[1] = mask;
	m_addresses.push_back(a);
}

