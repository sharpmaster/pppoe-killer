#ifndef _GHTTPMETHOD_H
#define _GHTTPMETHOD_H
#include <glib/config.h>
#include <bitset>
#include <ostream>
#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <glib/GLogger.h>
#include <glib/net/GHttpHeader.h>

namespace glib {
namespace net {
	
	class GLIB_EXPORT GHttpMethod : boost::noncopyable
	{
	public:
		static const int ERROR_CONNECT = -1;
		static const int ERROR_RESPONSE = -2;
		static std::string encodeURL(const std::string & url);
		
		GHttpMethod(const std::string & method);
		virtual ~GHttpMethod();

		void setHost(const std::string & host) {m_host = host;}
		std::string getHost() {return m_host;}
		void setPath(const std::string & path) {m_path = path;}
		std::string getPath() {return m_path;}
		
		int Execute();
		virtual boost::shared_ptr<char> getResponseBody(std::size_t & size);
		void addRequestHeader(const std::string & name, const std::string & value);
		void addRequestHeader(const glib::net::GHttpHeader & header);
		glib::net::GHttpHeader getResponseHeader(const std::string & name);

	protected:
		virtual std::string getRequestLine();
		virtual void writeRequestBody(std::ostream & request_stream);
		virtual void buildRequestHeaders();

		boost::ptr_map<std::string, glib::net::GHttpHeader> m_requestheader;
		boost::ptr_map<std::string, glib::net::GHttpHeader> m_responseheader;
		boost::shared_ptr<char> m_responsebody;
		std::size_t m_responsebody_size;
		boost::asio::io_service	m_ioservice;
		boost::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
	private:
		static std::vector<std::string> HEADER_SEQUENCE;
		static const char CMAP[16];
		static std::bitset<256> URLBIT;
		static std::bitset<256> INIT_URLBIT();
		
		void connect();
		void writeRequestLine(std::ostream & request_stream);
		void writeRequestHeaders(std::ostream & request_stream);

		std::string m_host;
		std::string m_path;
		std::string m_methodstr;
		
		boost::shared_ptr<GBaseLogger> m_logger;
	};
}}

#endif

