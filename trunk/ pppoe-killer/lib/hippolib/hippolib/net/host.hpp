#ifndef _HIPPOLIB_HOST_H
#define _HIPPOLIB_HOST_H

#include "subnet.hpp"
#include <vector>
#include <boost/asio.hpp>
#include <boost/foreach.hpp>
#include <boost/utility.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

namespace hippolib
{

	class host : boost::noncopyable
	{
	public:
		virtual ~host() {}
		virtual host* create(const std::string & expr) = 0;
		virtual host* clone() const = 0;
		virtual bool contains(const boost::asio::ip::address & addr) = 0;
		virtual bool contains(const std::string & host) = 0;
		virtual std::string to_string() = 0;
	};

	inline host* new_clone(const host & h) {return h.clone();}

	class iphost : public host
	{
	public:
		iphost() : m_addr(boost::asio::ip::address::from_string("127.0.0.1")) {}
		iphost(const boost::asio::ip::address & addr) : m_addr(addr) {}
		iphost(const iphost & h) {m_addr = h.m_addr;}
		
		iphost* create(const std::string & expr)
		{
			boost::system::error_code err;
			boost::asio::ip::address addr;

			addr = boost::asio::ip::address::from_string(expr, err);
			if(err)
				return NULL;

			return new iphost(addr);
		}

		iphost* clone() const
		{
			return new iphost(*this);
		}

		bool contains(const boost::asio::ip::address & addr)
		{
			return (m_addr == addr);
		}

		bool contains(const std::string & host)
		{
			return false;
		}

		std::string to_string()
		{
			return m_addr.to_string();
		}
		
	private:
		boost::asio::ip::address m_addr;
	};

	class nethost : public host
	{
	public:
		nethost() : m_network(boost::asio::ip::address_v4::from_string("127.0.0.1")),
			m_netmask(boost::asio::ip::address_v4::from_string("255.255.255.255")) {}
		nethost(const boost::asio::ip::address_v4 & net, const boost::asio::ip::address_v4 & mask)
			: m_network(net), m_netmask(mask) {}
		nethost(const nethost & n)
		{
			m_network = n.m_network;
			m_netmask = n.m_netmask;
		}

		nethost* create(const std::string & expr)
		{
			hippolib::subnet s;

			try
			{
				s = hippolib::subnet::parseSubnet(expr);
			}
			catch(...)
			{
				return NULL;
			}

			return new nethost(s.getNetwork(), s.getMask());
		}

		nethost* clone() const
		{
			return new nethost(*this);
		}

		bool contains(const boost::asio::ip::address & addr)
		{
			if(addr.is_v4() == false)
				return false;

			hippolib::subnet s(m_network, m_netmask);
			return s.containsIp(addr.to_v4());
		}

		bool contains(const std::string & host)
		{
			return false;
		}

		std::string to_string()
		{
			return (m_network.to_string() + "/" + m_netmask.to_string());
		}
		
	private:
		boost::asio::ip::address_v4 m_network;
		boost::asio::ip::address_v4 m_netmask;
	};

	class iprangehost : public host
	{
	public:
		iprangehost() : m_start(boost::asio::ip::address::from_string("127.0.0.1")),
						m_end(boost::asio::ip::address::from_string("127.0.0.1")) {}
		iprangehost(const boost::asio::ip::address & start, const boost::asio::ip::address & end)
			: m_start(start), m_end(end) {}
		iprangehost(const iprangehost & h)
		{
			m_start = h.m_start;
			m_end = h.m_end;
		}
		
		iprangehost* create(const std::string & expr)
		{
			std::string::size_type dotpos, dashpos;

			dotpos = expr.find_last_of('.');
			dashpos = expr.find_last_of('-');

			if(dotpos == std::string::npos || dashpos == std::string::npos ||
				dotpos >= (dashpos-1))
				return NULL;

			std::string start = expr.substr(0, dashpos);
			std::string end = expr.substr(0, dotpos+1) + expr.substr(dashpos+1);

			boost::system::error_code err;
			boost::asio::ip::address start_addr, end_addr;

			start_addr = boost::asio::ip::address::from_string(start, err);
			if(err)
				return NULL;
			end_addr = boost::asio::ip::address::from_string(end, err);
			if(err)
				return NULL;

			if(end < start)
				return NULL;

			return new iprangehost(start_addr, end_addr);
		}

		iprangehost* clone() const
		{
			return new iprangehost(*this);
		}

		bool contains(const boost::asio::ip::address & addr)
		{
			return ((m_start < addr || m_start == addr) && (addr < m_end || addr == m_end));
		}

		bool contains(const std::string & host)
		{
			return false;
		}

		std::string to_string()
		{
			return (m_start.to_string() + " - " + m_end.to_string());
		}
		
	private:
		boost::asio::ip::address m_start;
		boost::asio::ip::address m_end;
	};

	class namehost : public host
	{
	public:
		namehost() : m_regex(NULL), m_is_regex(false) {}
		namehost(const std::string & expr, const boost::regex *regex = NULL) : m_expr(expr)
		{
			if(regex == NULL)
			{
				m_regex = NULL;
				m_is_regex = false;
			}
			else
			{
				m_regex = new boost::regex(*regex);
				m_is_regex = true;
			}
		}
		namehost(const namehost & h)
		{
			m_expr = h.m_expr;
			if(h.m_regex == NULL)
			{
				m_regex = NULL;
				m_is_regex = false;
			}
			else
			{
				m_regex = new boost::regex(*(h.m_regex));
				m_is_regex = true;
			}
		}
		virtual ~namehost() {delete m_regex;}
		
		namehost* create(const std::string & expr)
		{
			if(expr.find('*') != std::string::npos) // wildcard charater is used
			{
				// replace . with \\., * with .*
				// for regular expression matching
				std::string output;
				boost::regex *regex = NULL;
				namehost *ret = NULL;

				output = boost::algorithm::replace_all_copy(expr, ".", "\\.");
				output = boost::algorithm::replace_all_copy(output, "*", ".*");
				try
				{
					regex = new boost::regex(output.c_str());
					ret = new namehost(expr, regex);
					delete regex;
				}
				catch(...)
				{
				}

				return ret;
			}
			else
			{
				return new namehost(expr, NULL);
			}

			return NULL;
		}

		namehost* clone() const
		{
			return new namehost(*this);
		}
		
		bool contains(const boost::asio::ip::address & addr)
		{
			return false;
		}

		bool contains(const std::string & host)
		{
			if(m_is_regex)
			{
				boost::cmatch what;
				return (boost::regex_match(host.c_str(), what, *m_regex));
			}
			else
			{
				return (m_expr == host);
			}

			return false;
		}

		std::string to_string()
		{
			return m_expr;
		}
		
	private:
		boost::regex *m_regex;
		bool m_is_regex;
		std::string m_expr;
	};
}

#endif

