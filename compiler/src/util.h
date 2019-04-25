#ifndef UTIL_INCLUDED
#define UTIL_INCLUDED

#include <libxml2/libxml/parser.h>
#include <string>

xmlAttrPtr xmlGetAttribute(xmlNodePtr node, const char * attr_name);
bool stobool(std::string str);
std::string xtos(const xmlChar* toString);

#endif