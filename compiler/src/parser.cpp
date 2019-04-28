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
//#include <libxml/parser.h>
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
//#          parseEnviroment(child);
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
  vector<xmlNodePtr> localRules;
  vector<xmlNodePtr> globalRules;
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
	abmodel.setDim(numOfDim);
	abmodel.setWrap(wrap);
	
	for (curNode; curNode; curNode = xmlNextElementSibling(curNode)) {
		//curNode = xmlNextElementSibling(curNode);
		if (curNode->name == "GlobalParameter") {
			xmlNodePtr temp = xmlFirstElementChild(curNode); // grabs the first child in global tag 
			for (temp; temp; temp = xmlNextElementSibling(temp)) { // accessing all global parameters
				char *tnm = (char*)xmlGetAttribute(temp, (const char *) "name")->children->content;           //->children->content; // temp name
				char *tvl = (char*)xmlGetAttribute(temp, (const char *) "value")->children->content; // temp value
				char *tty = (char*)xmlGetAttribute(temp, (const char *) "type")->children->content; // temp type
				bool cns = true; // temp
				if (tnm != NULL && tvl != NULL && tty != NULL) { // checking variable validity
					xmlNodePtr rule = xmlFirstElementChild(curNode); 
					if (rule != NULL) {
						cns = false;
						globalRules.push_back(rule);

					}
					abmodel.add_to_econtext(1, tnm, tvl,tty, cns);
				}
				else {
					std::cout << "INVALID GLOBAL ENVIRONMENT VARIABLE" << std::endl;
				}
			}
		} //
		else if (curNode->name == "localParameters") {
			xmlNodePtr temp = xmlFirstElementChild(curNode); // grabs the first child of the local paramters
			for (temp; temp; temp = xmlNextElementSibling(temp)) { // accessing all global parameters
				char* tnm =(char*) xmlGetAttribute(temp, (const char *) "name")->children->content; // temp name
				char* tvl = (char*)xmlGetAttribute(temp, (const char *) "value")->children->content; // temp value
				char* tty = (char*)xmlGetAttribute(temp, (const char *) "type")->children->content; // temp type
				bool cns = true;
				if (tnm != NULL && tvl != NULL && tty != NULL) { // checking variable validity
					xmlNodePtr rule = xmlFirstElementChild(curNode);
					if (rule != NULL) {
						cns = false;
						localRules.push_back(rule);
					}
					abmodel.add_to_econtext(0, tnm, tvl, tty, cns);
					abmodel.get_env_context().frames.at(1)->front()
				}
				else {
					std::cout << "INVALID LOCAL ENVIRONMENT VARIABLE" << std::endl;
				}
			}
		}
		
	}
	// parse rules after context is generated 
	for (vector<xmlNodePtr>::iterator git = globalRules.begin(); git != globalRules.end(); ++git) {
		//abmodel.add_GRAST(parseEnvRule(*git));
		
	}
	for (vector<xmlNodePtr>::iterator lit = localRules.begin(); lit != globalRules.end(); ++lit) {
		//abmodel.add_LRAST(parseEnvRule(*lit));
	}
}
// manages code parsing for env variables
// fairly sure that the full context is required to allow intra set referencing, so whole context goes through
std::unique_ptr<SourceAST> parseEnvRule(xmlNodePtr varRule) {
	return parse_logic(abmodel.get_env_context, varRule);

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

      toAdd.getQuestions().push_back(Question(toAdd.genContextBindings(), commsSearch));

      commsSearch = xmlNextElementSibling(commsSearch);
    }

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
        std::unique_ptr<SourceAST> logic_ast = parse_logic(ctxt, curNode);
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

xmlAttrPtr xmlGetAttribute(xmlNodePtr node, const char * attr_name) {
  xmlAttrPtr result = node->properties;
  while(result != NULL) {
    if(xmlStrcmp(result->name, (const xmlChar *)attr_name) == 0) {
      return result;
    }
    result = result->next;
  }
  return (xmlAttrPtr)NULL;
}

bool stobool(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    std::istringstream is(str);
    bool b;
    is >> std::boolalpha >> b;
    return b;
}

std::string xtos(const xmlChar* toString) { 
  return std::string((const char*)toString);
}

std::unique_ptr<SourceAST>
dispatch_on_logic_tag(const ContextBindings& ctxt, xmlNodePtr node)
{
  if(xmlStrcmp(node->name, (const xmlChar*)"if") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
      return std::make_unique<SourceAST_if_Verilog>(ctxt, node);
    }
    else {
      return std::make_unique<SourceAST_if_C>(ctxt, node);
    }
  }
  else if(xmlStrcmp(node->name, (const xmlChar*)"assign") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
      return std::make_unique<SourceAST_assignment_Verilog>(ctxt, node);
    }
    else {
      return std::make_unique<SourceAST_assignment_C>(ctxt, node);
    }
  }
  else if(xmlStrcmp(node->name, (const xmlChar*)"var") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
      return std::make_unique<SourceAST_var_Verilog>(ctxt, node);
    }
    else {
      return std::make_unique<SourceAST_var_C>(ctxt, node);
    }
  }
  else if(xmlStrcmp(node->name, (const xmlChar*)"operator") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
      return std::make_unique<SourceAST_operator_Verilog>(ctxt, node);
    }
    else {
      return std::make_unique<SourceAST_operator_C>(ctxt, node);
    }
  }
  else if(xmlStrcmp(node->name, (const xmlChar*)"constant") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
      return std::make_unique<SourceAST_constant_Verilog>(node);
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
parse_logic(const ContextBindings& ctxt, xmlNodePtr node)
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

