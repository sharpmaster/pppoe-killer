#ifndef _GTUNNELBASE_H
#define _GTUNNELBASE_H

#include <boost/asio.hpp>

namespace glib {
namespace net {

/**
* read asynchronously
* write synchronously
*/

class GLIB_EXPORT GTunnelBase
{
public:
	static const unsigned int DEFAULT_CLIENT_BUFFER_SIZE = 512;
	static const unsigned int DEFAULT_SERVER_BUFFER_SIZE = 512;
	
	GTunnelBase(const std::string & client_initial_buffer = "",
					const std::string & server_initial_buffer = "",
					unsigned int clientbufsz = DEFAULT_CLIENT_BUFFER_SIZE,
					unsigned int serverbufsz = DEFAULT_SERVER_BUFFER_SIZE)
				: m_client_initial_buffer(client_initial_buffer), m_server_initial_buffer(server_initial_buffer),
				m_client_readbufsz(clientbufsz), m_server_readbufsz(serverbufsz),
				m_client_readbuf(new char[m_client_readbufsz]), m_server_readbuf(new char[m_server_readbufsz]),
				m_server_pending_read(false), m_server_pending_write(false),
				m_client_pending_read(false), m_client_pending_write(false) {}
	
	virtual ~GTunnelBase()
	{
		delete m_client_readbuf;
		delete m_server_readbuf;
	}

	virtual void Initialize()
	{
		if(m_client_initial_buffer.size() > 0 && m_client_initial_buffer.size() < m_client_readbufsz)
		{
			std::copy(m_client_initial_buffer.begin(), m_client_initial_buffer.end(), m_client_readbuf);
			write_server(m_client_initial_buffer.size());
		}
		else
			async_read_client();
		
		if(m_server_initial_buffer.size() > 0 && m_server_initial_buffer.size() < m_server_readbufsz)
		{
			std::copy(m_server_initial_buffer.begin(), m_server_initial_buffer.end(), m_server_readbuf);
			write_client(m_server_initial_buffer.size());
		}
		else
			async_read_server();
	}
	
	virtual boost::asio::ip::address getClientAddress() = 0;
	virtual boost::asio::ip::address getServerAddress() = 0;
	virtual void CloseServer() = 0;
	virtual void CloseClient() = 0;
	
protected:
	virtual void async_read_server() = 0;
	virtual void async_read_client() = 0;
	virtual void write_server(std::size_t bytes_transferred) = 0;
	virtual void write_client(std::size_t bytes_transferred) = 0;
	virtual void handle_client_read(const boost::system::error_code& error, std::size_t bytes_transferred)
	{
		if(!error)
		{
			m_server_pending_write = true;
			write_server(bytes_transferred);
		}
		else
			CloseClient();
		m_client_pending_read = false;
	}
	virtual void handle_server_read(const boost::system::error_code& error, std::size_t bytes_transferred)
	{
		if(!error)
		{
			m_client_pending_write = true;
			write_client(bytes_transferred);
		}
		else
			CloseServer();
		m_server_pending_read = false;
	}
	virtual void handle_client_write(const boost::system::error_code & error)
	{
		if(!error)
		{
			m_server_pending_read = true;
			async_read_server();
		}
		else
			CloseClient();
		m_client_pending_write = false;
	}
	virtual void handle_server_write(const boost::system::error_code & error)
	{
		if(!error)
		{
			m_client_pending_read = true;
			async_read_client();
		}
		else
			CloseServer();
		m_server_pending_write = false;
	}

	std::string m_client_initial_buffer;
	std::string m_server_initial_buffer;
	unsigned int m_client_readbufsz;
	unsigned int m_server_readbufsz;
	char* m_client_readbuf;
	char* m_server_readbuf;
	bool m_server_pending_read;
	bool m_server_pending_write;
	bool m_client_pending_read;
	bool m_client_pending_write;
};

}
}

#endif

