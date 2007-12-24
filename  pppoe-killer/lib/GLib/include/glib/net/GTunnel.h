#ifndef _GTUNNEL_H
#define _GTUNNEL_H
#include <glib/config.h>
#include <glib/net/GTunnelBase.h>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

namespace glib {
namespace net {

	class GLIB_EXPORT GTunnel : glib::net::GTunnelBase
	{
	public:
		static const unsigned int DEFAULT_CLIENT_BUFFER_SIZE = 512;
		static const unsigned int DEFAULT_SERVER_BUFFER_SIZE = 512;

		GTunnel(const std::string & client_initial_buffer = "",
					const std::string & server_initial_buffer = "",
					unsigned int clientbufsz = DEFAULT_CLIENT_BUFFER_SIZE,
					unsigned int serverbufsz = DEFAULT_SERVER_BUFFER_SIZE);
		GTunnel(boost::shared_ptr<boost::asio::ip::tcp::socket> client,
					boost::shared_ptr<boost::asio::ip::tcp::socket> server,
					const std::string & client_initial_buffer = "",
					const std::string & server_initial_buffer = "",
					unsigned int clientbufsz = DEFAULT_CLIENT_BUFFER_SIZE,
					unsigned int serverbufsz = DEFAULT_SERVER_BUFFER_SIZE);
		virtual ~GTunnel();

		virtual boost::asio::ip::address getClientAddress() {return m_client_endpoint.address();}
		virtual boost::asio::ip::address getServerAddress() {return m_server_endpoint.address();}
		virtual void CloseServer();
		virtual void CloseClient();
		
		boost::asio::ip::tcp::endpoint getClientEndpoint() {return m_client_endpoint;}
		boost::asio::ip::tcp::endpoint getServerEndpoint() {return m_server_endpoint;}

		void setServerSocket(boost::shared_ptr<boost::asio::ip::tcp::socket> s, boost::system::error_code & ret);
		void setClientSocket(boost::shared_ptr<boost::asio::ip::tcp::socket> s, boost::system::error_code & ret);
		void Close();
		bool isServerClosed() {return m_server_closed;}
		bool isClientClosed() {return m_client_closed;}
		
	protected:
		virtual void async_read_server();
		virtual void async_read_client();
		virtual void write_server(std::size_t bytes_transferred);
		virtual void write_client(std::size_t bytes_transferred);
		virtual void handle_client_read(const boost::system::error_code & error, std::size_t bytes_transferred);
		virtual void handle_server_read(const boost::system::error_code & error, std::size_t bytes_transferred);
		virtual void handle_client_write(const boost::system::error_code & error);
		virtual void handle_server_write(const boost::system::error_code & error);
		
	private:
		bool m_server_closed;
		bool m_client_closed;
		boost::shared_ptr<boost::asio::ip::tcp::socket> m_clientsocket;
		boost::shared_ptr<boost::asio::ip::tcp::socket> m_serversocket;
		/**
		* Extract the endpoints from the 2 sockets
		* Then endpoint information will be available after socket is closed
		*/
		boost::asio::ip::tcp::endpoint m_client_endpoint;
		boost::asio::ip::tcp::endpoint m_server_endpoint;

		boost::mutex m_server_close_mutex;
		boost::mutex m_client_close_mutex;
	};

}}

#endif

