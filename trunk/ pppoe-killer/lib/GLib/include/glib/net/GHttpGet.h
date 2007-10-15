#ifndef _GHTTPGET_H
#define _GHTTPGET_H
#include <glib/config.h>
#include <glib/net/GHttpMethod.h>

namespace glib {
namespace net {
	
	class GLIB_EXPORT GHttpGet : public glib::net::GHttpMethod
	{
	public:
		GHttpGet();
		GHttpGet(const std::string & url);

		virtual std::string getRequestLine();
		std::string getUrl() {return m_url;}
		void setUrl(const std::string & url);
		std::string getQueryString() {return m_querystring;}
		void setQueryString(const std::string & str) {m_querystring = str;}
	private:
		std::string m_url;
		std::string m_querystring;
	};
}}

#endif

