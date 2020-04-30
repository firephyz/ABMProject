/*
 * GAMP PARSER
 * 3.9.19
 */
#include "parser.h"
#include "abmodel.h"
#include "config.h"
#include "util.h"
#include "comms.h"
#include "debug.h"
#include "source_tree/source_c.h"
#include "source_tree/source_verilog.h"

using namespace std;
#include <libxml2/libxml/parser.h>
#include <string>

#include <iomanip>
#include <cctype>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <memory>

extern ABModel abmodel;
extern struct program_args_t pargs;

ABModel& parse_model(const char * xml_model_path)
{
    xmlDocPtr inputDoc = xmlReadFile(xml_model_path, NULL, 0x0);
    if (inputDoc == NULL) {
        std::cerr << "Couldn't read input file! \'"
                  << xml_model_path << "\'." << std::endl;
        exit(-1);
    }

    xmlNodePtr root = xmlDocGetRootElement(inputDoc);

    // Get the model name or set to default
    auto model_name = xmlGetAttribute(root, "name");
    if (model_name != NULL) {
    	abmodel.model_name = std::string((const char*)model_name->children->content);
    } else {
    	abmodel.model_name = std::string("DEFAULT_NAME");
    }

    xmlNodePtr child = xmlFirstElementChild(root);

    // init state must be called after parseAgents so create a callback
    // to manage time dependency
    bool parsed_agents = false;
    std::pair<void (*)(xmlNodePtr child), xmlNodePtr> init_state_callback;

    while (child != NULL) {
      if (xmlStrcmp(child->name, (const xmlChar*)"environment") == 0){
        // change parser state so we limit the allowed AST
        parser.set_state(ParserState::Environment);
        parseEnviroment(child);
      } else if(xmlStrcmp(child->name, (const xmlChar*)"agentDefinitions") == 0){
        parseAgents(child);
        parsed_agents = true;

        // Now parse init state if its been waiting
        if (init_state_callback.first) {
          init_state_callback.first(init_state_callback.second);
        }
      } else if(xmlStrcmp(child->name, (const xmlChar*)"initialState") == 0) {
        if (parsed_agents) {
          parseInitialState(child);
        }
        else {
          init_state_callback.first = parseInitialState;
          init_state_callback.second = child;
        }
      } else {
        std::cout << "OOPS" << std::endl;
      }

      child = xmlNextElementSibling(child);
    }

    return abmodel;
}

void parseInitialState(xmlNodePtr node)
{
  xmlNodePtr curNode = xmlFirstElementChild(node);
  while(curNode != NULL) {
    if(xmlStrcmp(curNode->name, (const xmlChar *)"dimensions") == 0) {
      parse_dimensions(curNode);
    }
    else if(xmlStrcmp(curNode->name, (const xmlChar *)"agent") == 0) {
      parse_init_agent(curNode);
    }
    else {
      std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Unrecognized xml tag \'" << (const char *)curNode->name << "\'." << std::endl;
      exit(-1);
    }

    curNode = xmlNextElementSibling(curNode);
  }

  // Do final sorting and pointer linking
  abmodel.init.sortInitAgents();
  abmodel.init.resolveInitAgentLinks();

  #if DEBUG
  for(auto iter = abmodel.init.agents_by_type.begin(); iter != abmodel.init.agents_by_type.end(); ++iter) {
    std::cout << *iter << ", ";
  }
  std::cout << std::endl;

  for(auto iter = abmodel.init.agents_by_position.begin(); iter != abmodel.init.agents_by_position.end(); ++iter) {
    std::cout << *iter << ", ";
  }
  std::cout << std::endl;
  #endif
}

void parse_init_agent(xmlNodePtr node)
{
  // push back indicies into init.agents vector
  abmodel.init.agents_by_type.emplace_back(abmodel.init.agents_by_type.size());
  abmodel.init.agents_by_position.emplace_back(abmodel.init.agents_by_position.size());

  // push back new agent
  abmodel.init.agents.emplace_back(node);

  // increment agent type
  auto& ag = abmodel.init.agents[abmodel.init.agents.size() - 1];
  if (ag.type_unique_id >= abmodel.init.num_agents_by_type.size()) {
    abmodel.init.num_agents_by_type.emplace_back(0);
  }
  abmodel.init.num_agents_by_type[ag.type_unique_id]++;

  // set init data index for indexing into type init data array
  ag.setTypeRegionIndex(abmodel.init.num_agents_by_type[ag.type_unique_id] - 1);
}

void parse_dimensions(xmlNodePtr curNode)
{
  static bool parsed_dimensions = false;

  if(parsed_dimensions) {
    std::cerr << "Only put one \'dimensions\' tag in an \'initialState\' tag." << std::endl;
    exit(-1);
  }
  else {
    parsed_dimensions = true;
    xmlAttrPtr xml_attr = xmlGetAttribute(curNode, "sizes");
    if(xml_attr == NULL) {
      std::cerr << "\'dimensions\' tag is missing the \'sizes\' attribute." << std::endl;
      exit(-1);
    }

    std::string dim_sizes_str((const char *)xml_attr->children->content);
    size_t str_pos = 0;
    while(str_pos < dim_sizes_str.length()) {
      size_t new_str_pos = dim_sizes_str.find(" ", str_pos);
      if(new_str_pos == std::string::npos) new_str_pos = dim_sizes_str.length();
      std::string dim_size_str = dim_sizes_str.substr(str_pos, new_str_pos - str_pos);
      abmodel.dimension_sizes.push_back(std::stoi(dim_size_str));
      str_pos = new_str_pos + 1;
    }
  }
}

void parseEnviroment(xmlNodePtr envChild) {
  auto value_str = (const char*)xmlGetAttribute(envChild, (const char*)"relationType")->children->content;
//  uint test_dim_count = 0;
  if (value_str == NULL) {
  	std::cerr << "No relational type specified for enivroment: " << envChild->name;
  }
  xmlNodePtr curNode = NULL;

  std::string relationType(value_str);
  abmodel.relationType = relationType;

  if (relationType == "spatial") {
		curNode = xmlFirstElementChild(envChild);
  	if (xmlStrcmp(curNode->name, (const xmlChar*)("spatialRelation")) == 0)     {
      xmlAttrPtr numOfDim_node = xmlGetAttribute(curNode, "dimensions");
      if (numOfDim_node == NULL) {
        std::cerr << "Error missing dimensions attr" << std::endl;
      }
      abmodel.numOfDimensions = std::stoi(std::string((const char*)numOfDim_node->children->content));
		} else {
    	std::cerr << "Error, mismatched relation definition" << std::endl;
	  }
  } else {
		std::cerr << "Error, other relatiion types are not supported at this time" << std::endl;
   }

  // // Check if the number of dimensions match the number supplied
  // if (abmodel.numOfDimensions != test_dim_count) {
	// 	std::cerr << "Number of dimensions specified does not match the number of those supplied to enviroment tag" << std::endl;
	// }
  //
  // // Check if the number of inital dimension sizes matches the number supplied
  // if (abmodel.numOfDimensions != abmodel.init.dimension_sizes.size()) {
	// 	std::cerr << "Number of dimensions specified does not match the number of initial dimensions supplied" << std::endl;
	// }
}

void parseAgents(xmlNodePtr agentsChild) {
  xmlNodePtr curNode = NULL;

  curNode = xmlFirstElementChild(agentsChild);

  // While there are more agents to parse keep calling newAgentDef();
  while (curNode  != NULL) {
    newAgentDef(curNode);
    curNode = xmlNextElementSibling(curNode);
  }

  // Resolve question and agent pointers in each answer
  // and answer and agent pointers in each question
  for(auto& data : parser.answers_to_be_linked) {
    data.resolve_answer_links();
  }
  parser.answers_to_be_linked.clear();

  // Make sure every question has an answer
  bool did_fail = false;
  for(auto& agent : abmodel.agents) {
    for(auto& q : agent.getQuestions()) {
      if(q->getAnswer() == nullptr) {
        std::cerr << "Error: Question \'" << q->get_name() << "\' in agent \'" << agent.getName() << "\'" << " does not have an answer link." << std::endl;
        did_fail = true;
      }
    }
  }
  if(did_fail) {
    exit(-1);
  }
}

// Every answer has a corresponding question so we will link
// questions to the answer in this function as well.
void
ParserObject::answer_link_data::resolve_answer_links()
{
  const AgentForm& agent = abmodel.find_agent_by_name(agent_name);

  // find the question to link to the answer
  auto q_iter = std::find_if(
    agent.getQuestions().begin(),
    agent.getQuestions().end(),
    [&](const std::shared_ptr<Question>& q) {
      return question_name == q->get_name();
    });
  if(q_iter == agent.getQuestions().end()) {
    std::cerr << "Error during answer linking. Could not find question \'" << question_name << "\'" << std::endl;
    exit(-1);
  }
  Question& question = *(q_iter->get());

  // resolve links on answer side
  answer->set_agent(agent);
  answer->set_question(question);

  // resolve links on question side
  question.set_agent(agent);
  question.set_answer(*answer);
}

void newAgentDef(xmlNodePtr agent) {
  xmlNodePtr curNode = agent;

  // Check that the first tag is Agent and go ahead and grab the name
  if (xmlStrcmp(curNode->name, (const xmlChar*)"agent") == 0) {
    std::string name((const char*)(xmlGetAttribute(curNode, "type")->children->content));
    abmodel.agents.emplace_back(name);
    AgentForm& toAdd = abmodel.agents[abmodel.agents.size() - 1];

    // Check for log attr for agent if not present preset to false and type check
  	auto xml_log_attr = xmlGetAttribute(curNode, "log_en");
  	if(xml_log_attr == NULL) {
  		 toAdd.log_en = false;
  	} else {
   		std::string log_en((const char*)xml_log_attr->children->content);

    	if (log_en == "true") {
      	toAdd.log_en = true;
     	} else if (log_en == "false") {
      	toAdd.log_en = false;
  		} else {
  			// Type error
  		std::cerr << "Improper logging attr type (Agent)" << std::endl;
  		}
    }

    // Get Agent Vars
    curNode = xmlFirstElementChild(curNode);
    if(xmlStrcmp(curNode->name, (const xmlChar*)"agentScope")) {
      std::cerr << "Improper Agent Definition: Missing Agent Scope" << std::endl;
      return; // Return error
    }
    parseBindings(toAdd.getAgentScopeBindingsMut(), curNode, SymbolBindingScope::AgentLocal);
    for(auto& binding : toAdd.getAgentScopeBindingsMut()) {
      binding.set_agent_index(abmodel.agents.size() - 1);
    }

    // Get the agent states
    parser.set_state(ParserState::States);
    curNode = xmlNextElementSibling(curNode);
    if(xmlStrcmp(curNode->name, (const xmlChar*)"rules") != 0) {
      std::cerr << "Missing rules tag in agent definition" << std::endl;
      exit(-1);
    }

    xmlNodePtr state_node = xmlFirstElementChild(curNode);
    while(state_node != NULL) {
      parseAgentState(toAdd, abmodel.agents.size() - 1, state_node);
      state_node = xmlNextElementSibling(state_node);
    }

    // Get the Comms interface
    curNode = xmlNextElementSibling(curNode);
    if(xmlStrcmp(curNode->name, (const xmlChar *)"commsInterface") != 0) {
      std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Agent is missing its \'CommsInterface\' tag." << std::endl;
      exit(-1);
    }
    auto xml_attr = xmlGetAttribute(curNode, "neighborhood");
    if(xml_attr == NULL) {
      util::error(curNode) << "Comms interface does not have a neighborhood type." << std::endl;
      exit(-1);
    }
    toAdd.set_neighborhood(parse_neighborhood(abmodel.num_agents(), (const char *)xml_attr->children->content));

    xmlNodePtr commsSearch = xmlFirstElementChild(curNode);
    while (commsSearch != NULL) {
      if(xmlStrcmp(commsSearch->name, (const xmlChar*)"question") == 0) {
        toAdd.getQuestionsMut().push_back(std::make_shared<Question>(toAdd.genContextBindings(), commsSearch));
      }
      else if(xmlStrcmp(commsSearch->name, (const xmlChar*)"answer") == 0) {
        toAdd.getAnswersMut().push_back(std::make_shared<Answer>(toAdd.genContextBindings(), commsSearch));
      }
      else {
        util::error(curNode) << "Expecting a \'question\' tag node but received a \'" << commsSearch->name << "\' tag node." << std::endl;
        exit(-1);
      }

      commsSearch = xmlNextElementSibling(commsSearch);
    }

    // Resolve ask-question links in state logic ast
    toAdd.resolve_ask_links();
  } else {
    std::cerr << "Improper Agent Definition: Missing Agent Tag" << std::endl;
    exit(-1);
  }
}

std::unique_ptr<CommsNeighborhood>
parse_neighborhood(const size_t agent_index, const char * n)
{
  std::string str(n);

  size_t underscore_loc = str.find("_", 0);

  // TODO implement more neighborhoods
  if(underscore_loc == str.npos) {
    std::cerr << "Parsing unimplemented or unrecognized neighborhood." << std::endl;
    exit(-1);
  }
  else {
    std::string first = str.substr(0, underscore_loc);
    std::string second = str.substr(underscore_loc + 1);

    if(first == "ncube") {
      int size = std::stoi(second);
      return std::make_unique<Comms::NCube>(agent_index, size);
    }
  }

  // shouldn't get here
  return std::unique_ptr<CommsNeighborhood>(nullptr);
}

void parseAgentState(AgentForm& agent, size_t agent_index, xmlNodePtr curNode) {

  if (xmlStrcmp(curNode->name, (const xmlChar*)"state") == 0) {
    std::string name((const char*)xmlGetAttribute(curNode, "name")->children->content);
    StateInstance& newState = agent.add_state(StateInstance(name));

    curNode = xmlFirstElementChild(curNode);
    while(curNode != NULL) {
      // Get the state variables
      if(xmlStrcmp(curNode->name, (const xmlChar*)"stateScope") == 0) {
        parseBindings(newState.getStateScopeBindingsMut(), curNode, SymbolBindingScope::StateLocal);

        // Link the bindings we just created with this state so we can use the info later
        for(auto& binding : newState.getStateScopeBindingsMut()) {
          binding.set_agent_index(agent_index);
          binding.set_state_index(agent.getStates().size() - 1);
        }
      } else if (xmlStrcmp(curNode->name, (const xmlChar*)"logic") == 0) {
// TODO the references in this context may be invalidated if the agent is moved in its vector
        const ContextBindings ctxt = agent.genContextBindings(newState);
        std::unique_ptr<SourceAST> logic_ast = parser.parse_logic(ctxt, curNode);
        newState.add_logic(std::move(logic_ast));
        logic_ast.release();
      }

      curNode = xmlNextElementSibling(curNode);
    }
  } else {
    std::cout << "Invalid State tag: " << curNode->name << std::endl;
  }
}

void parseBindings(std::vector<SymbolBinding>& bindings, xmlNodePtr curNode, SymbolBindingScope scope) {
   curNode = xmlFirstElementChild(curNode);

   while (curNode != NULL) {
    struct VariableType varType;
    std::string val;
    bool is_constant;

    auto xml_attr = xmlGetAttribute(curNode, "type");
    if(xml_attr == NULL) {
      util::error(curNode) << "Variable declaration is missing its \'type\' attribute." << std::endl;
      exit(-1);
    }

    std::string type_str = std::string((const char*)(xml_attr->children->content));
    varType.type = strToEnum(type_str);
    std::string symName = (const char*)(xmlGetAttribute(curNode, "id")->children->content);
    if(symName.find("-") != std::string::npos) {
      util::error(curNode) << "Variables cannot use the dash \'-\' in their \'id\' attribute." << std::endl;
      exit(-1);
    }

		auto xml_log_attr = xmlGetAttribute(curNode, "log");
		if(xml_log_attr == NULL) {
		 varType.log_en = false;
		} else {
 			std::string log_en((const char*)xml_log_attr->children->content);

  	  if (log_en == "true") {
    		varType.log_en = true;
   	  } else if (log_en == "false") {
    	 	varType.log_en = false;
			} else {
				// error
			std::cerr << "Improper logging attr type" << std::endl;
		  }
    }

    // Check if the var has a val attribute and if so use that else set default
    xml_attr = xmlGetAttribute(curNode, "value");
    if (xml_attr == NULL) {
      val = std::string(); // empty so code-gen assumes default
    } else {
      val = std::string((const char *)xml_attr->children->content);
    }

    // Check if the var has a is_constant attribute and if so use that else set default
    if (xmlGetAttribute(curNode, "is_constant") != NULL) {
      is_constant = stobool((const char*)(xmlGetAttribute(curNode, "is_constant")->children->content));
    } else {
      is_constant = false;
    }
    bindings.emplace_back(symName, varType, val, is_constant, scope);

    curNode = xmlNextElementSibling(curNode);
   }
}

std::unique_ptr<SourceAST>
dispatch_on_logic_tag(const ContextBindings& ctxt, xmlNodePtr node)
{
  if(xmlStrcmp(node->name, (const xmlChar*)"if") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
//      return std::make_unique<SourceAST_if_Verilog>(ctxt, node);
    }
    else {
      return std::make_unique<SourceAST_if_C>(ctxt, node);
    }
  }
  else if(xmlStrcmp(node->name, (const xmlChar*)"assign") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
//      return std::make_unique<SourceAST_assignment_Verilog>(ctxt, node);
    }
    else {
      return std::make_unique<SourceAST_assignment_C>(ctxt, node);
    }
  }
  else if(xmlStrcmp(node->name, (const xmlChar*)"var") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
//      return std::make_unique<SourceAST_var_Verilog>(ctxt, node);
    }
    else {
      return std::make_unique<SourceAST_var_C>(ctxt, node);
    }
  }
  else if(xmlStrcmp(node->name, (const xmlChar*)"operator") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
//      return std::make_unique<SourceAST_operator_Verilog>(ctxt, node);
    }
    else {
      return std::make_unique<SourceAST_operator_C>(ctxt, node);
    }
  }
  else if(xmlStrcmp(node->name, (const xmlChar*)"constant") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
//      return std::make_unique<SourceAST_constant_Verilog>(node);
    }
    else {
      return std::make_unique<SourceAST_constant_C>(node);
    }
  }
  else if(xmlStrcmp(node->name, (const xmlChar*)"return") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
//      return std::make_unique<SourceAST_return_Verilog>(ctxt, node);
    }
    else {
      return std::make_unique<SourceAST_return_C>(ctxt, node);
    }
  }
  else if(xmlStrcmp(node->name, (const xmlChar*)"response") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
//      return std::make_unique<SourceAST_response_Verilog>(ctxt, node);
    }
    else {
      return std::make_unique<SourceAST_response_C>(ctxt, node);
    }
  }
  else {
    std::cerr << "Unknown xml tag \'" << node->name << "\' in \'<logic>\' block.\n";
    exit(-1);
  }

  // shouldn't get here
  return std::unique_ptr<SourceAST>(nullptr);
}

std::unique_ptr<SourceAST>
ParserObject::parse_logic(const ContextBindings& ctxt, xmlNodePtr node)
{
  xmlNodePtr child = xmlFirstElementChild(node);
  std::unique_ptr<SourceAST> result = dispatch_on_logic_tag(ctxt, child);
  std::unique_ptr<SourceAST> * last_node = &result;

  child = xmlNextElementSibling(child);
  while(child != NULL) {
    (*last_node)->append_next(dispatch_on_logic_tag(ctxt, child));
    last_node = &(*last_node)->next;

    child = xmlNextElementSibling(child);
  }
  (*last_node)->append_next(std::unique_ptr<SourceAST>(nullptr));

  return result;
}

#if VERBOSE_AST_GEN
std::string
ParserState_to_string(ParserState s)
{
  switch(s) {
    case ParserState::States:
      return std::string("States");
    case ParserState::Questions:
      return std::string("Questions");
    case ParserState::Answers:
      return std::string("Answers");
    case ParserState::Environment:
      return std::string("Environment");
  }
  return std::string("UNKNOWN ParserState");
}
#endif
