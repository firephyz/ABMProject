#ifndef PARSER_INCLUDED
#define PARSER_INCLUDED

#include "abmodel.h"
#include "compiler_types.h"

#include <libxml2/libxml/parser.h>
#include <vector>

ABModel& parse_model(const char * xml_model_path);
void parseEnviroment(xmlNodePtr envChild);
void parseAgents(xmlNodePtr agentsChild);
void newAgentDef(xmlNodePtr agent);
xmlAttrPtr xmlGetAttribute(xmlNodePtr node, const char * attr_name);
bool stobool(std::string str);
std::string xtos(xmlChar* toString);

std::unique_ptr<SourceAST> parse_logic(xmlNodePtr node);
void parseBindings(std::vector<SymbolBinding>& bindings, xmlNodePtr curNode); 
void parseAgentStates(AgentForm& agent, xmlNodePtr curNode);

#endif
