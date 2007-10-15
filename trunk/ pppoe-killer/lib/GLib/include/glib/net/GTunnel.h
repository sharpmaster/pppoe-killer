#ifndef _GTUNNEL_H
#define _GTUNNEL_H
#include <glib/config.h>
#include <boost/asio.hpp>
#include <boost/signal.hpp>
#include <boost/signals/slot.hpp>
#include <boost/thread/mutex.hpp>

namespace glib {
namespace net {
	
	class GLIB_EXPORT GTunnel
	{
	public:
		static const unsigned int DEFAULT_CLIENT_BUFFER_SIZE = 512;
		static const unsigned int DEFAULT_SERVER_BUFFER_SIZE = 512;
		
		GTunnel(boost::shared_ptr<boost::asio::ip::tcp::socket> client,
					boost::shared_ptr<boost::asio::ip::tcp::socket> server,
					unsigned int clientbufsz = DEFAULT_CLIENT_BUFFER_SIZE,
					unsigned int serverbufsz = DEFAULT_SERVER_BUFFER_SIZE,
					std::string * ini_clientbuf = NULL);
		virtual ~GTunnel();

		boost::asio::ip::tcp::endpoint getClientEndpoint() {return m_client_endpoint;}
		boost::asio::ip::tcp::endpoint getServerEndpoint() {return m_server_endpoint;}

		void Close();
		bool isClosed() {return m_closed;}
		void addCloseListener(const boost::signal2<void, const boost::asio::ip::tcp::endpoint &, const boost::asio::ip::tcp::endpoint &>::slot_type & slot)
		{msig_closed.connect(slot);}
	protected:
		virtual void handle_client_read(const boost::asio::error& error, std::size_t bytes_transferred);
		virtual void handle_server_read(const boost::asio::error& error, std::size_t bytes_transferred);
		virtual void handle_client_write(const boost::asio::error& error, std::size_t bytes_transferred);
		virtual void handle_server_write(const boost::asio::error& error, std::size_t bytes_transferred);
	private:
		char* m_client_readbuf;
		char* m_server_readbuf;
		unsigned int m_client_readbufsz;
		unsigned int m_server_readbufsz;
		bool m_closed;
		boost::shared_ptr<boost::asio::ip::tcp::socket> m_clientsocket;
		boost::shared_ptr<boost::asio::ip::tcp::socket> m_serversocket;
		boost::signal2<void, const boost::asio::ip::tcp::endpoint &, const boost::asio::ip::tcp::endpoint &> msig_closed;
		/**
		* Extract the endpoints from the 2 sockets
		* Then endpoint information will be available after socket is closed
		*/
		boost::asio::ip::tcp::endpoint m_client_endpoint;
		boost::asio::ip::tcp::endpoint m_server_endpoint;

		boost::mutex m_closemutex;
	};

}}

#endif

