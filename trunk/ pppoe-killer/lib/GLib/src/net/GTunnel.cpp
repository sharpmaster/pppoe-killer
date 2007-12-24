#include <iostream>
#include <glib/GThread.h>
#include <glib/net/GTunnel.h>
#include <boost/bind.hpp>

using namespace std;
using namespace glib::net;

GTunnel::GTunnel(const std::string & client_initial_buffer,
					const std::string & server_initial_buffer,
					unsigned int clientbufsz, unsigned int serverbufsz)
					: GTunnelBase(client_initial_buffer, server_initial_buffer, clientbufsz, serverbufsz),
					m_server_closed(false), m_client_closed(false)
{
}

GTunnel::GTunnel(boost::shared_ptr<boost::asio::ip::tcp::socket> client,
					boost::shared_ptr<boost::asio::ip::tcp::socket> server,
					const std::string & client_initial_buffer,
					const std::string & server_initial_buffer,
					unsigned int clientbufsz, unsigned int serverbufsz)
					: GTunnelBase(client_initial_buffer, server_initial_buffer, clientbufsz, serverbufsz),
					m_server_closed(false), m_client_closed(false)
{
	m_clientsocket = client;
	m_serversocket = server;
}

GTunnel::~GTunnel()
{
	Close();
}

void GTunnel::setServerSocket(boost::shared_ptr<boost::asio::ip::tcp::socket> s, boost::system::error_code & ret)
{
	boost::asio::ip::tcp::endpoint point = s->remote_endpoint(ret);

	if(ret)
	{
		return;
	}
	
	CloseServer();

	unsigned int waittime = 0;
	while((m_server_pending_read == true || m_server_pending_write == true) &&
			waittime < 1000)
	{
			GThread::sleep(100);
			waittime += 100;
	}

	m_serversocket = s;
	m_server_endpoint = point;
}

void GTunnel::setClientSocket(boost::shared_ptr<boost::asio::ip::tcp::socket> s, boost::system::error_code & ret)
{
	boost::asio::ip::tcp::endpoint point = s->remote_endpoint(ret);

	if(ret)
	{
		return;
	}
	
	CloseClient();

	unsigned int waittime = 0;
	while((m_client_pending_read == true || m_client_pending_write == true) &&
			waittime < 1000)
	{
			GThread::sleep(100);
			waittime += 100;
	}

	m_clientsocket = s;
	m_client_endpoint = point;
}

void GTunnel::async_read_server()
{
	m_serversocket->async_read_some(boost::asio::buffer(m_server_readbuf, m_server_readbufsz),
										boost::bind(&GTunnel::handle_server_read, this,
										boost::asio::placeholders::error,
										boost::asio::placeholders::bytes_transferred));
}

void GTunnel::async_read_client()
{
	m_clientsocket->async_read_some(boost::asio::buffer(m_client_readbuf, m_client_readbufsz),
										boost::bind(&GTunnel::handle_client_read, this,
										boost::asio::placeholders::error,
										boost::asio::placeholders::bytes_transferred));
}

void GTunnel::write_server(std::size_t bytes_transferred)
{
	boost::system::error_code err;
	boost::asio::write(*m_serversocket, boost::asio::buffer(m_client_readbuf, bytes_transferred),
						boost::asio::transfer_all(), err);
	handle_server_write(err);
}

void GTunnel::write_client(std::size_t bytes_transferred)
{
	boost::system::error_code err;
	boost::asio::write(*m_clientsocket, boost::asio::buffer(m_server_readbuf, bytes_transferred),
						boost::asio::transfer_all(), err);
	handle_client_write(err);
}

void GTunnel::handle_client_read(const boost::system::error_code & error, std::size_t bytes_transferred)
{
	GTunnelBase::handle_client_read(error, bytes_transferred);
}

void GTunnel::handle_server_read(const boost::system::error_code & error, std::size_t bytes_transferred)
{
	GTunnelBase::handle_server_read(error, bytes_transferred);
}

void GTunnel::handle_client_write(const boost::system::error_code & error)
{
	GTunnelBase::handle_client_write(error);
}

void GTunnel::handle_server_write(const boost::system::error_code & error)
{
	GTunnelBase::handle_server_write(error);
}
		
void GTunnel::CloseClient()
{
	boost::mutex::scoped_lock lock(m_client_close_mutex);
	boost::system::error_code err;
	if(m_client_closed)
		return;
	m_clientsocket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
	m_clientsocket->close(err);
	m_client_closed = true;
}

void GTunnel::CloseServer()
{
	boost::mutex::scoped_lock lock(m_server_close_mutex);
	boost::system::error_code err;
	if(m_server_closed)
		return;
	m_serversocket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
	m_serversocket->close(err);
	m_server_closed = true;
}

void GTunnel::Close()
{
	CloseClient();
	CloseServer();
}

