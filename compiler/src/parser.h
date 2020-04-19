#ifndef PARSER_INCLUDED
#define PARSER_INCLUDED

//#include "abmodel.h"
#include "comms.h"
#include "debug.h"

#include <libxml2/libxml/parser.h>
#include <vector>
#include <memory>
#include <iostream>

#ifndef VERBOSE_AST_GEN
#error "Code which has the potential to utilize VERBOSE_AST_GEN debug\
 option does not have it defined."
#endif

class SourceAST_ask;
class SourceAST;
class ABModel;
class AgentForm;
class ContextBindings;
class Answer;

// used in parser object to know what sort of thing its parsing
enum class ParserState {
  States,
  Questions,
  Answers,
  Environment,
};

// need this if we want to print ParserState enum classes
#if VERBOSE_AST_GEN
  std::string ParserState_to_string(ParserState s);
#endif

// Parsing needs to know extra information like the general xml area within
// which it is called. Always call parser.set_state when you're about to parse_logic
// in an entirely new area that has different parse rules, syntax, ect.
struct ParserObject {
  ParserState state;

  // nodes that need to be linked back with their Question objects. The AST is
  // parsed before the Questions
  std::vector<SourceAST_ask *> nodes;

  // Answers that need to be linked to corresponding questions and agent targets
  struct answer_link_data {
    Answer * answer;
    std::string question_name;
    std::string agent_name;

    answer_link_data(Answer * answer, std::string& question_name, std::string agent_name)
      : answer(answer), question_name(question_name), agent_name(agent_name)
    {}
    void resolve_answer_links();
  };
  std::vector<struct answer_link_data> answers_to_be_linked;

  std::unique_ptr<SourceAST> parse_logic(const ContextBindings& ctxt, xmlNodePtr node);
  void set_state(ParserState s) {
    state = s;

    #if VERBOSE_AST_GEN
      std::cout << "Setting parser state: " << ParserState_to_string(s) << std::endl;
    #endif
  }
};

#include "source_tree/source_ast.h"
#include "compiler_types.h"
class SymbolBinding;
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
std::unique_ptr<CommsNeighborhood> parse_neighborhood(const size_t agent_index, const char * n);

std::unique_ptr<SourceAST> dispatch_on_logic_tag(const ContextBindings& ctxt, xmlNodePtr node);
void parseBindings(std::vector<SymbolBinding>& bindings, xmlNodePtr curNode, SymbolBindingScope scope);
void parseAgentState(AgentForm& agent, size_t agent_index, xmlNodePtr curNode);

#endif
