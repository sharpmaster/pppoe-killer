#ifndef _GHTTPSTATUS_H
#define _GHTTPSTATUS_H
#include <glib/config.h>
#include <boost/assign/list_of.hpp>
#include <string>
#include <map>

namespace glib {
namespace net {
	
	class GLIB_EXPORT GHttpStatus
	{
	public:
		static std::string getStatusText(int code);

		static const int SC_CONTINUE = 100;
		static const int SC_SWITCHING_PROTOCOLS = 101;
		static const int SC_PROCESSING = 102;
		
		static const int SC_OK = 200;
		static const int SC_CREATED = 201;
		static const int SC_ACCEPTED = 202;
		static const int SC_NON_AUTHORITATIVE_INFORMATION = 203;
		static const int SC_NO_CONTENT = 204;
		static const int SC_RESET_CONTENT = 205;
		static const int SC_PARTIAL_CONTENT = 206;
		static const int SC_MULTI_STATUS = 207;
		
		static const int SC_MULTIPLE_CHOICES = 300;
		static const int SC_MOVED_PERMANENTLY = 301;
		static const int SC_MOVED_TEMPORARILY = 302;
		static const int SC_SEE_OTHER = 303;
		static const int SC_NOT_MODIFIED = 304;
		static const int SC_USE_PROXY = 305;
		static const int SC_TEMPORARY_REDIRECT = 307;
		
		static const int SC_BAD_REQUEST = 400;
		static const int SC_UNAUTHORIZED = 401;
		static const int SC_PAYMENT_REQUIRED = 402;
		static const int SC_FORBIDDEN = 403;
		static const int SC_NOT_FOUND = 404;
		static const int SC_METHOD_NOT_ALLOWED = 405;
		static const int SC_NOT_ACCEPTABLE = 406;
		static const int SC_PROXY_AUTHENTICATION_REQUIRED = 407;
		static const int SC_REQUEST_TIMEOUT = 408;
		static const int SC_CONFLICT = 409;
		static const int SC_GONE = 410;
		static const int SC_LENGTH_REQUIRED = 411;
		static const int SC_PRECONDITION_FAILED = 412;
		static const int SC_REQUEST_TOO_LONG = 413;
		static const int SC_REQUEST_URI_TOO_LONG = 414;
		static const int SC_UNSUPPORTED_MEDIA_TYPE = 415;
		static const int SC_REQUESTED_RANGE_NOT_SATISFIABLE = 416;
		static const int SC_EXPECTATION_FAILED = 417;
		static const int SC_INSUFFICIENT_SPACE_ON_RESOURCE = 419;
		static const int SC_METHOD_FAILURE = 420;
		static const int SC_UNPROCESSABLE_ENTITY = 422;
		static const int SC_LOCKED = 423;
		static const int SC_FAILED_DEPENDENCY = 424;

		static const int SC_INTERNAL_SERVER_ERROR = 500;
		static const int SC_NOT_IMPLEMENTED = 501;
		static const int SC_BAD_GATEWAY = 502;
		static const int SC_SERVICE_UNAVAILABLE = 503;
		static const int SC_GATEWAY_TIMEOUT = 504;
		static const int SC_HTTP_VERSION_NOT_SUPPORTED = 505;

		static const int SC_INSUFFICIENT_STORAGE = 507;

		static std::map<int, std::string> REASON_PHRASES;
	};
}}

#endif

