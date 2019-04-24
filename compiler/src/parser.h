#ifndef PARSER_INCLUDED
#define PARSER_INCLUDED

#include "abmodel.h"
#include "compiler_types.h"

#include <libxml2/libxml/parser.h>
#include <vector>

class ParserContext {
public:
  std::vector<std::vector<SymbolBinding>> bindings;
};

ABModel& parse_model(const char * xml_model_path);
void parseInitialState(xmlNodePtr node);
void parseEnviroment(xmlNodePtr envChild);
void parseAgents(xmlNodePtr agentsChild);
void newAgentDef(xmlNodePtr agent);
void parse_dimensions(xmlNodePtr curNode);
void parse_init_agent(xmlNodePtr node);

std::unique_ptr<SourceAST> parse_logic(const ContextBindings& ctxt, xmlNodePtr node);
std::unique_ptr<SourceAST> dispatch_on_logic_tag(const ContextBindings& ctxt, xmlNodePtr node);
void parseBindings(std::vector<SymbolBinding>& bindings, xmlNodePtr curNode); 
void parseAgentStates(AgentForm& agent, xmlNodePtr curNode);

#endif
