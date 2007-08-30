#ifndef _LIBXMLUTIL_H
#define _LIBXMLUTIL_H

#include <string>

/**
* Apache Portable Runtime XML library
* It's very simple, but it's not suitable for generating XML documents
*/
#include <apr_xml.h>

std::string aprXmlGetPropString(apr_xml_elem* e, const char* attr, const char* def);
bool aprXmlGetPropBool(apr_xml_elem* e, const char* attr, bool def);


#endif

