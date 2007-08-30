#include "libxml_util.h"

std::string aprXmlGetPropString(apr_xml_elem* e, const char* attr, const char* def)
{
	apr_xml_attr *a;

	if(e == NULL) return std::string(def);
	if(e->attr == NULL) return std::string(def);
	
	a = e->attr;
	while(a)
	{
		if(!strcmp(a->name, attr))
			return std::string(a->value);
		a = a->next;
	}

	return std::string(def);
}

bool aprXmlGetPropBool(apr_xml_elem* e, const char* attr, bool def)
{
	apr_xml_attr *a;

	if(e == NULL) return def;
	if(e->attr == NULL) return def;

	a = e->attr;
	while(a)
	{
		if(!strcmp(a->name, attr))
		{
			if(!strcmp(a->value, "1"))
				return true;
			else
				return false;
		}
		a = a->next;
	}

	return def;
}

