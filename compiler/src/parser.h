#ifndef PARSER_INCLUDED
#define PARSER_INCLUDED

#include "abmodel.h"

#include <libxml2/libxml/parser.h>

ABModel& parse_model(const char * xml_model_path);
void parseEnviroment(xmlNodePtr envChild);
void parseAgents(xmlNodePtr agentsChild);
void newAgentDef(xmlNodePtr agent);
xmlAttrPtr xmlGetAttribute(xmlNodePtr node, const char * attr_name);
bool stobool(std::string str);
std::string xtos(xmlChar* toString);

#endif
