#include <glib/net/GTunnel.h>
#include <boost/bind.hpp>

using namespace std;
using namespace glib::net;

GTunnel::GTunnel(boost::shared_ptr<boost::asio::ip::tcp::socket> client,
					boost::shared_ptr<boost::asio::ip::tcp::socket> server,
					unsigned int clientbufsz, unsigned int serverbufsz,
					std::string *  ini_clientbuf)
					: m_closed(false), m_client_readbufsz(clientbufsz), m_server_readbufsz(serverbufsz)
{
	m_clientsocket = client;
	m_serversocket = server;
	m_client_endpoint = m_clientsocket->remote_endpoint(boost::asio::ignore_error());
	m_server_endpoint = m_serversocket->remote_endpoint(boost::asio::ignore_error());
	m_client_readbuf = new char[m_client_readbufsz];
	m_server_readbuf = new char[m_server_readbufsz];

	if(ini_clientbuf == NULL)
	{
		m_clientsocket->async_read_some(boost::asio::buffer(m_client_readbuf, m_client_readbufsz),
										boost::bind(&GTunnel::handle_client_read, this,
													boost::asio::placeholders::error,
													boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		boost::asio::async_write(*m_serversocket,
						boost::asio::buffer(*ini_clientbuf),
						boost::bind(&GTunnel::handle_server_write, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}
	
	m_serversocket->async_read_some(boost::asio::buffer(m_server_readbuf, m_server_readbufsz),
		boost::bind(&GTunnel::handle_server_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

GTunnel::~GTunnel()
{
	Close();
	delete m_client_readbuf;
	delete m_server_readbuf;
}

void GTunnel::handle_client_read(const boost::asio::error & error, std::size_t bytes_transferred)
{
	if(!error)
	{
		boost::asio::async_write(*m_serversocket,
						boost::asio::buffer(m_client_readbuf, bytes_transferred),
						boost::bind(&GTunnel::handle_server_write, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}
	else
		Close();
}

void GTunnel::handle_client_write(const boost::asio::error & error, std::size_t bytes_transferred)
{
	if(!error)
	{
		m_serversocket->async_read_some(boost::asio::buffer(m_server_readbuf, m_server_readbufsz),
		boost::bind(&GTunnel::handle_server_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
	}
	else
		Close();
}

void GTunnel::handle_server_read(const boost::asio::error & error, std::size_t bytes_transferred)
{
	if(!error)
	{
		boost::asio::async_write(*m_clientsocket,
						boost::asio::buffer(m_server_readbuf, bytes_transferred),
						boost::bind(&GTunnel::handle_client_write, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}
	else
		Close();
}

void GTunnel::handle_server_write(const boost::asio::error & error, std::size_t bytes_transferred)
{
	if(!error)
	{
		m_clientsocket->async_read_some(boost::asio::buffer(m_client_readbuf, m_client_readbufsz),
		boost::bind(&GTunnel::handle_client_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
	}
	else
		Close();
}

void GTunnel::Close()
{
	boost::mutex::scoped_lock lock(m_closemutex);
	
	if(m_closed == true)
		return;
	
	m_clientsocket->close(boost::asio::ignore_error());
	m_serversocket->close(boost::asio::ignore_error());
	msig_closed(m_client_endpoint, m_server_endpoint);
	m_closed = true;
}

