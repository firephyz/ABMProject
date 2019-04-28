/*
 * GAMP PARSER
 * 3.9.19
 */
#include "parser.h"
#include "abmodel.h"
#include "config.h"
#include "source_c.h"
#include "source_verilog.h"
#include "util.h"
#include "comms.h"

using namespace std;
#include <libxml2/libxml/parser.h>
#include <string>

#include <iomanip>
#include <cctype>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <algorithm>

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
    xmlNodePtr child = xmlFirstElementChild(root);

    while (child != NULL) {
        if (xmlStrcmp(child->name, (const xmlChar*)"environment") == 0){
          // change parser state so we limit the allowed AST
          parser.set_state(ParserState::Environment);
//        parseEnviroment(child);
        } else if(xmlStrcmp(child->name, (const xmlChar*)"agentDefinitions") == 0){
          parseAgents(child);
        } else if(xmlStrcmp(child->name, (const xmlChar*)"initialState") == 0) {
          parseInitialState(child);
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

  // sort initial_agents by position so they are processed in the correct order
  std::sort(abmodel.init.agents.begin(), abmodel.init.agents.end());
}

void parse_init_agent(xmlNodePtr node)
{
  abmodel.init.agents.emplace_back(node);
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
      abmodel.init.dimension_sizes.push_back(std::stoi(dim_size_str));

      str_pos = new_str_pos + 1;
    }

    // TODO make sure number of dimension sizes matches the number of dimensions
  }
}

void parseEnviroment(xmlNodePtr envChild) {
  const char * value_str = (const char*)xmlGetAttribute(envChild, (const char*)"relationType")->children->content;
  xmlNodePtr curNode = NULL;
  int numOfDim = 0;
	bool wrap = 0;

	if (value_str != NULL) {
    std::string relationType(value_str);
    if(relationType == "spatial") {
		  curNode = xmlFirstElementChild(envChild);
			if (xmlStrcmp(curNode->name, (const xmlChar*)"spatialRelation") == 0){
                     	numOfDim = std::stoi((const char*)xmlGetAttribute(curNode, "dimensions")->children->content, NULL, 10);
				wrap = stobool((const char*)(xmlGetAttribute(curNode, "wrap")->children->content)); // <====8
				std::cout << numOfDim << "\n" << wrap << std::endl;
			} else {
				std::cout << "Invalid Enviroment Definiton" << std::endl;
      }
		}
	}
}

void parseAgents(xmlNodePtr agentsChild) {
  xmlNodePtr curNode = NULL;

  curNode = xmlFirstElementChild(agentsChild);

  // While there are more agents to parse keep calling newAgentDef();
  while (curNode  != NULL) {
    newAgentDef(curNode);
    curNode = xmlNextElementSibling(curNode);
  }
}

void newAgentDef(xmlNodePtr agent) {
  xmlNodePtr curNode = agent;

  // Check that the first tag is Agent and go ahead and grab the name
  if (xmlStrcmp(curNode->name, (const xmlChar*)"agent") == 0) {
    std::string name((const char*)(xmlGetAttribute(curNode, "type")->children->content));
    AgentForm toAdd(name);

    // Get Agent Vars
    curNode = xmlFirstElementChild(curNode);
    if(xmlStrcmp(curNode->name, (const xmlChar*)"agentScope")) {
      std::cerr << "Improper Agent Definition: Missing Agent Scope" << std::endl;
      return; // Return error
    }
    parseBindings(toAdd.getAgentScopeBindings(), curNode);

    // Get the agent states
    parser.set_state(ParserState::States);
    curNode = xmlNextElementSibling(curNode);
    if(xmlStrcmp(curNode->name, (const xmlChar*)"rules") != 0) {
      std::cerr << "Missing rules tag in agent definition" << std::endl;
      exit(-1);
    }
    parseAgentStates(toAdd, xmlFirstElementChild(curNode));

    // Get the Comms interface
    curNode = xmlNextElementSibling(curNode);
    if(xmlStrcmp(curNode->name, (const xmlChar *)"commsInterface") != 0) {
      std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Agent is missing its \'CommsInterface\' tag." << std::endl;
      exit(-1);
    }
    auto xml_attr = xmlGetAttribute(curNode, "neighborhood");
    if(xml_attr == NULL) {
      std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Comms interface does not have a neighborhood type." << std::endl;
      exit(-1);
    }
    toAdd.set_neighborhood(parse_neighborhood(abmodel.num_agents(), (const char *)xml_attr->children->content));

    xmlNodePtr commsSearch = xmlFirstElementChild(curNode);
    while (commsSearch != NULL) {
      if(xmlStrcmp(commsSearch->name, (const xmlChar*)"Question") != 0) {
        std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Expecting a \'Question\' tag node but received a \'" << commsSearch->name << "\' tag node." << std::endl;
        exit(-1);
      }

      toAdd.getQuestions().push_back(std::make_shared<Question>(toAdd.genContextBindings(), commsSearch));

      commsSearch = xmlNextElementSibling(commsSearch);
    }

    // Resolve ask-question links
    for(SourceAST_ask * node : parser.nodes) {
      for(auto& q : toAdd.getQuestions()) {
        if(q->get_name() == node->getQuestionName()) {
          // give ask node the shared_ptr to Question
          node->setQuestion(q);
          break;
        }
      }

      if(node->getQuestion() == nullptr) {
        std::cerr << "Could not link ask node with question name \'" << node->getQuestionName();
        std::cerr << "\' with a question in agent \'" << toAdd.getName() << "\'." << std::endl;
        exit(-1);
      }
    }
    // empty nodes for next agent to be parsed
    parser.nodes.clear();

    abmodel.add_agent(toAdd);
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

  if(underscore_loc == str.npos) {

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

void parseAgentStates(AgentForm& agent, xmlNodePtr curNode) {

  if (xmlStrcmp(curNode->name, (const xmlChar*)"state") == 0) {
    std::string name((const char*)xmlGetAttribute(curNode, "name")->children->content);
    StateInstance& newState = agent.add_state(StateInstance(name));

    curNode = xmlFirstElementChild(curNode);
    while(curNode != NULL) {
      // Get the state variables
      if(xmlStrcmp(curNode->name, (const xmlChar*)"stateScope") == 0) {
        parseBindings(newState.getStateScopeBindings(), curNode);
      } else if (xmlStrcmp(curNode->name, (const xmlChar*)"logic") == 0) {
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

void parseBindings(std::vector<SymbolBinding>& bindings, xmlNodePtr curNode) {
   curNode = xmlFirstElementChild(curNode);

   while (curNode != NULL) {
    struct VariableType varType;
    std::string val;
    bool is_constant;

    varType.type = strToEnum((const char*)(xmlGetAttribute(curNode, "type")->children->content));
    std::string symName = (const char*)(xmlGetAttribute(curNode, "id")->children->content);
    
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
    xmlAttrPtr xml_attr = xmlGetAttribute(curNode, "value");
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
    bindings.emplace_back(symName, varType, val, is_constant);

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

  return std::move(result);
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