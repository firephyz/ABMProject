/*
 * GAMP PARSER 
 * 3.9.19
 */
#include "parser.h"
#include "abmodel.h"
#include "config.h"
#include "source_c.h"
#include "source_verilog.h"

using namespace std;
#include "libxml/parser.h"
#include <istream>
#include <string>

#include <iomanip>
#include <algorithm>
#include <cctype>
#include <vector>
#include <iostream>
#include <stdlib.h>

ABModel abmodel;
extern struct program_args_t pargs;

ABModel& parse_model(const char * xml_model_path)
{
    xmlDocPtr inputDoc = xmlReadFile(xml_model_path, NULL, 0x0);
    if (inputDoc == NULL) {
        std::cerr << "Couldn't read input file! \'" 
                  << xml_model_path << "\'." << std::endl;
        exit(-1);
    }
    std::cout << "File loaded in!" << std::endl; 
  
    xmlNodePtr root = xmlDocGetRootElement(inputDoc);
    xmlNodePtr child = xmlFirstElementChild(root);   

    while (child != NULL) { 
        if (xmlStrcmp(child->name, (const xmlChar*)"environment") == 0){ 
//#          parseEnviroment(child);
        } else if(xmlStrcmp(child->name, (const xmlChar*)"agentDefinitions") == 0){ 
          parseAgents(child);
        } else {
          std::cout << "OOPS" << std::endl;
        }

       child = xmlNextElementSibling(child);      
    }
    
    /* Simple Test 
    while (child != NULL) {
        std::cout << child->name << std::endl; 
        child = xmlNextElementSibling(child);
    }
    */

    return abmodel;
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
				}
				else {
					std::cout << "INVALID LOCAL ENVIRONMENT VARIABLE" << std::endl;
				}
			}
		}
		
	}
	// parse rules after context is generated 
	for (vector<xmlNodePtr>::iterator git = globalRules.begin(); git != globalRules.end(); ++git) {
		abmodel.add_GRAST(parseEnvRule(*git));
	}
	for (vector<xmlNodePtr>::iterator lit = localRules.begin(); lit != globalRules.end(); ++lit) {
		abmodel.add_LRAST(parseEnvRule(*lit));
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
  std::string curAttr = "";  
  
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
    curNode = xmlFirstElementChild(curNode);
    parseAgentStates(toAdd, curNode);

    abmodel.add_agent(toAdd);
  } else {
    std::cerr << "Improper Agent Definition: Missing Agent Tag" << std::endl;
    exit(-1);
  }
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

  curNode = xmlNextElementSibling(curNode);
}

void parseBindings(std::vector<SymbolBinding>& bindings, xmlNodePtr curNode) {
   curNode = xmlFirstElementChild(curNode);
       
   while (curNode != NULL) {
    struct VariableType varType;
    void* val;
    bool is_constant;  
       
    varType.type = strToEnum((const char*)(xmlGetAttribute(curNode, "type")->children->content));  
    std::string symName = (const char*)(xmlGetAttribute(curNode, "id")->children->content);
    
    // Check if the var has a val attribute and if so use that else set default
    if (xmlGetAttribute(curNode, "val") != NULL) {
      // TODO must parse data from attribute content
      //val = xmlGetAttribute(curNode, "val")->children->content;
      val = NULL;
    } else { 
      val = NULL;
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
    std::transform(str.begin(), str.end(), str.begin(), tolower);
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
  else {
    std::cerr << "Unknown xml tag \'" << node->name << "\' in \'<logic>\' block.\n";
    exit(-1);
  }
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

  return std::move(result);
}

