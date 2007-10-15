#include <glib/net/GHttpStatus.h>

using namespace std;
using namespace glib::net;

map<int, string> GHttpStatus::REASON_PHRASES =
	boost::assign::map_list_of
		(GHttpStatus::SC_OK, "OK")
		(GHttpStatus::SC_CREATED, "Created")
		(GHttpStatus::SC_ACCEPTED, "Accepted")
		(GHttpStatus::SC_NO_CONTENT, "No Content")
		(GHttpStatus::SC_MOVED_PERMANENTLY, "Moved Permanently")
		(GHttpStatus::SC_MOVED_TEMPORARILY, "Moved Temporarily")
		(GHttpStatus::SC_NOT_MODIFIED, "Not Modified")
		(GHttpStatus::SC_BAD_REQUEST, "Bad Request")
		(GHttpStatus::SC_UNAUTHORIZED, "Unauthorized")
		(GHttpStatus::SC_FORBIDDEN, "Forbidden")
		(GHttpStatus::SC_NOT_FOUND, "Not Found")
		(GHttpStatus::SC_INTERNAL_SERVER_ERROR, "Internal Server Error")
		(GHttpStatus::SC_NOT_IMPLEMENTED, "Not Implemented")
		(GHttpStatus::SC_BAD_GATEWAY, "Bad Gateway")
		(GHttpStatus::SC_SERVICE_UNAVAILABLE, "Service Unavailable")
		(GHttpStatus::SC_CONTINUE, "Continue")
		(GHttpStatus::SC_TEMPORARY_REDIRECT, "Temporary Redirect")
		(GHttpStatus::SC_METHOD_NOT_ALLOWED, "Method Not Allowed")
		(GHttpStatus::SC_CONFLICT, "Conflict")
		(GHttpStatus::SC_PRECONDITION_FAILED, "Precondition Failed")
		(GHttpStatus::SC_REQUEST_TOO_LONG, "Request Too Long")
		(GHttpStatus::SC_REQUEST_URI_TOO_LONG, "Request-URI Too Long")
		(GHttpStatus::SC_UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type")
		(GHttpStatus::SC_MULTIPLE_CHOICES, "Multiple Choices")
		(GHttpStatus::SC_SEE_OTHER, "See Other")
		(GHttpStatus::SC_USE_PROXY, "Use Proxy")
		(GHttpStatus::SC_PAYMENT_REQUIRED, "Payment Required")
		(GHttpStatus::SC_NOT_ACCEPTABLE, "Not Acceptable")
		(GHttpStatus::SC_PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required")
		(GHttpStatus::SC_REQUEST_TIMEOUT, "Request Timeout")
		(GHttpStatus::SC_SWITCHING_PROTOCOLS, "Switching Protocols")
		(GHttpStatus::SC_NON_AUTHORITATIVE_INFORMATION, "Non Authoritative Information")
		(GHttpStatus::SC_RESET_CONTENT, "Reset Content")
		(GHttpStatus::SC_PARTIAL_CONTENT, "Partial Content")
		(GHttpStatus::SC_GATEWAY_TIMEOUT, "Gateway Timeout")
		(GHttpStatus::SC_HTTP_VERSION_NOT_SUPPORTED, "Http Version Not Supported")
		(GHttpStatus::SC_GONE, "Gone")
		(GHttpStatus::SC_LENGTH_REQUIRED, "Length Required")
		(GHttpStatus::SC_REQUESTED_RANGE_NOT_SATISFIABLE, "Requested Range Not Satisfiable")
		(GHttpStatus::SC_EXPECTATION_FAILED, "Expectation Failed")
		(GHttpStatus::SC_PROCESSING, "Processing")
		(GHttpStatus::SC_MULTI_STATUS, "Multi-Status")
		(GHttpStatus::SC_UNPROCESSABLE_ENTITY, "Unprocessable Entity")
		(GHttpStatus::SC_INSUFFICIENT_SPACE_ON_RESOURCE, "Insufficient Space On Resource")
		(GHttpStatus::SC_METHOD_FAILURE, "Method Failure")
		(GHttpStatus::SC_LOCKED, "Locked")
		(GHttpStatus::SC_INSUFFICIENT_STORAGE , "Insufficient Storage")
		(GHttpStatus::SC_FAILED_DEPENDENCY, "Failed Dependency");
string GHttpStatus::getStatusText(int code)
{
	map<int, string>::iterator i = REASON_PHRASES.find(code);
	if(i != REASON_PHRASES.end())
		return i->second;
	return string("N/A");
}
