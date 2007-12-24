#include <glib/net/GHttpMethod.h>
#include <boost/regex.hpp>
#include <boost/array.hpp>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace glib;
using namespace glib::net;
using boost::asio::ip::tcp;

GHttpMethod::GHttpMethod(const string & method) : m_methodstr(method)
{
	m_logger.reset(GLogger::getLogger("glib_ghttp"));
	m_socket.reset(new tcp::socket(m_ioservice));
	m_responsebody.reset((char*)NULL);
	m_responsebody_size = 0;
}

GHttpMethod::~GHttpMethod()
{
	m_socket->close();
}

void GHttpMethod::addRequestHeader(const string & name, const string & value)
{
	string n(name);
	m_requestheader.insert(n, new GHttpHeader(name, value));
}

void GHttpMethod::addRequestHeader(const GHttpHeader & header)
{
	string name = header.getName();
	m_requestheader.insert(name, new GHttpHeader(header));
}

GHttpHeader GHttpMethod::getResponseHeader(const string & name)
{
	boost::ptr_map<std::string, GHttpHeader>::iterator i = m_responseheader.find(name);
	if(i == m_responseheader.end())
		throw invalid_argument("No such header");

	return *i->second;
}

int GHttpMethod::Execute()
{
	try
	{
		connect();
	}
	catch(boost::system::error_code & e)
	{
		GERROR(*m_logger)("Connect error: %s", e.message().c_str());
		return ERROR_CONNECT;
	}

	buildRequestHeaders();
	m_responsebody.reset((char*)NULL);
	m_responsebody_size = 0;

	boost::asio::streambuf request;
	ostream request_stream(&request);
	
	writeRequestLine(request_stream);
	writeRequestHeaders(request_stream);
	writeRequestBody(request_stream);

	boost::asio::write(*m_socket, request);

	// Read the response status line.
	boost::asio::streambuf response;
	boost::asio::read_until(*m_socket, response, boost::regex("\r\n"));

	// Check that response is OK.
	istream response_stream(&response);
	string http_version;
	response_stream >> http_version;
	unsigned int status_code;
	response_stream >> status_code;
	string status_message;
	getline(response_stream, status_message);
	if (!response_stream || http_version.substr(0, 5) != "HTTP/")
	{
		GERROR(*m_logger)("Invalid response: %s %d %s", http_version.c_str(), status_code, status_message.c_str());
		return ERROR_RESPONSE;
	}

	GDEBUG(*m_logger)("Status received: %s %d %s", http_version.c_str(), status_code, status_message.c_str());

	// Read the response headers, which are terminated by a blank line.
	boost::asio::read_until(*m_socket, response, boost::regex("\r\n\r\n"));
	string header;
	m_responseheader.clear();
	while(getline(response_stream, header) && header != "\r")
	{
		GHttpHeader ghdr = GHttpHeader::parseString(header);
		GDEBUG(*m_logger)("Header received: %s: %s", ghdr.getName().c_str(), ghdr.getValue().c_str());

		string name = ghdr.getName();
		m_responseheader.insert(name, new GHttpHeader(ghdr));
	}

	return status_code;
}

void GHttpMethod::connect()
{
	tcp::resolver resolver(m_ioservice);
	tcp::resolver::query query(m_host, "http");
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::resolver::iterator end;

	boost::system::error_code error = boost::asio::error::host_not_found;
	while (error && endpoint_iterator != end)
	{
		m_socket->close();
		m_socket->connect(*endpoint_iterator++, error);
	}
	
	if (error)
		throw error;
}

void GHttpMethod::buildRequestHeaders()
{
	if(m_requestheader.find("Host") == m_requestheader.end())
	{
		string host = "Host";
		m_requestheader.insert(host, new GHttpHeader("Host", m_host));
	}

	string connection = "Connection";
	m_requestheader.insert(connection, new GHttpHeader("Connection", "close"));
	if(m_requestheader.find("Accept") == m_requestheader.end())
	{
		string accept = "Accept";
		m_requestheader.insert(accept, new GHttpHeader("Accept", "*/*"));
	}
}

string GHttpMethod::getRequestLine()
{
	string ret;

	ret = m_methodstr + " " + m_path + " HTTP/1.1";
	return ret;
}

void GHttpMethod::writeRequestLine(ostream & request_stream)
{
	string line = getRequestLine();
	
	request_stream << line << "\r\n";
}

void GHttpMethod::writeRequestHeaders(ostream & request_stream)
{
	boost::ptr_map<string, GHttpHeader>::iterator i;
	map<string, string> headers;
	map<string, string>::iterator hi;

	for(i = m_requestheader.begin(); i != m_requestheader.end(); i++)
		headers.insert(make_pair(i->first, i->second->getString()));

	BOOST_FOREACH(string h, HEADER_SEQUENCE)
	{
		hi = headers.find(h);
		if(hi != headers.end())
		{
			request_stream << hi->second << "\r\n";
			headers.erase(hi);
		}
	}
	
	for(hi = headers.begin(); hi != headers.end(); hi++)
		request_stream << hi->second << "\r\n";

	request_stream << "\r\n";
}

void GHttpMethod::writeRequestBody(ostream & request_stream)
{
}

boost::shared_ptr<char> GHttpMethod::getResponseBody(size_t & size)
{
	if(m_responsebody_size > 0)
		return m_responsebody;
	
	boost::array<char, 1024> buffer;
	char *buf;
	size_t now_size = 0, max_size = 4096;

	buf = new char[max_size];
	
	while(true)
	{
		try
		{
			size_t n = m_socket->read_some(boost::asio::buffer<char, 1024>(buffer));
			if(now_size + n > max_size)
			{
				max_size += 4096;
				char* newbuf = new char[max_size];
				memcpy(newbuf, buf, now_size);
				delete buf;
				buf = newbuf;
			}
			memcpy(&buf[now_size], buffer.data(), n);
			now_size += n;
		}
		catch (boost::system::error_code & e)
		{
			if(e != boost::asio::error::eof)
				GERROR(*m_logger)("Get response error: %s", e.message().c_str());
			break;
		}
	}

	if(now_size == 0)
	{
		delete buf;
		m_responsebody.reset((char*)NULL);
		m_responsebody_size = 0;
	}
	else
	{
		m_responsebody.reset(buf);
		m_responsebody_size = now_size;
	}
	size = now_size;
	
	return m_responsebody;
}

string GHttpMethod::encodeURL(const string & url)
{
	stringbuf sbuf;

	BOOST_FOREACH(char c, url)
	{
		if(URLBIT[(size_t)c] == true)
		{
			if(c == ' ')
				sbuf.sputc('+');
			else
				sbuf.sputc(c);
		}
		else
		{
			sbuf.sputc('%');
			sbuf.sputc(CMAP[(int)((c >> 4) & 0xF)]);
			sbuf.sputc(CMAP[(int)(c & 0xF)]);
		}
	}

	return sbuf.str();
}
vector<string> GHttpMethod::HEADER_SEQUENCE =
	boost::assign::list_of("Host")("User-Agent")("Accept")("Accept-Language")("Accept-Encoding")("Accept-Charset");
const char GHttpMethod::CMAP[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
bitset<256> GHttpMethod::URLBIT = GHttpMethod::INIT_URLBIT();
bitset<256> GHttpMethod::INIT_URLBIT()
{
	bitset<256> ret;

	for(size_t i = (size_t)'a'; i <= (size_t)'z'; i++)
		ret[i] = true;
	
	for(size_t i = (size_t)'A'; i <= (size_t)'Z'; i++)
		ret[i] = true;

	for(size_t i = (size_t)'0'; i <= (size_t)'9'; i++)
		ret[i] = true;

	ret[(size_t)'-'] = true;
	ret[(size_t)'_'] = true;
	ret[(size_t)'.'] = true;
	ret[(size_t)'*'] = true;
	ret[(size_t)' '] = true;
	
	return ret;
}

