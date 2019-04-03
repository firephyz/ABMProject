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
#include <libxml2/libxml/parser.h>
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
  std::string curAttr = "";  
  
  // Check that the first tag is Agent and go ahead and grab the name
  if (xmlStrcmp(curNode->name, (const xmlChar*)"agent") == 0) { 
    std::string name((const char*)(xmlGetAttribute(curNode, "type")->children->content));
    AgentForm toAdd(name); 

    xmlNodePtr root = curNode; // Set the root to the first child of the agen definition  
    
    // Get Agent Vars
    curNode = xmlFirstElementChild(curNode); 
    xmlNodePtr placeHolder = curNode; // Set the placeholder to the first child of the agent definition      
    if(xmlStrcmp(curNode->name, (const xmlChar*)"agentScope")) { 
      std::cerr << "Improper Agent Definition: Missing Agent Scope" << std::endl; 
      return; // Return error     
    }    
    parseBindings(toAdd.getAgentScopeBindings(), curNode);
     
    // Get the agent states 
    curNode = placeHolder;
    curNode = xmlNextElementSibling(curNode);
    placeHolder = curNode;  
    parseAgentStates(toAdd, curNode);

    abmodel.add_agent(toAdd);
  } else {
    std::cerr << "Improper Agent Definition: Missing Agent Tag" << std::endl;
  }
} 

void parseAgentStates(AgentForm& agent, xmlNodePtr curNode) {
  while (curNode != NULL) {
    if (xmlStrcmp(curNode->name, (const xmlChar*)"state")) {
      std::string name((const char*)xmlGetAttribute(curNode, "name")->children->content); 
      
      xmlNodePtr depthNode = xmlFirstElementChild(curNode);

      StateInstance newState(name);
      
      while(depthNode != NULL) {
        // Get the state variables  
        if(xmlStrcmp(depthNode->name, (const xmlChar*)"stateScope")) {
          parseBindings(newState.getStateScopeBindings(), depthNode);   
        } else if (xmlStrcmp(depthNode->name, (const xmlChar*)"logic")) {
          std::unique_ptr<SourceAST> logic_ast = parse_logic(curNode);
          newState.add_logic(logic_ast);
        }

        agent.add_state(newState);
        
        depthNode = xmlNextElementSibling(depthNode);
      }
    } else {
      std::cout << "Invalid State tag: " << curNode->name << std::endl;
    }
  }
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
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    std::istringstream is(str);
    bool b;
    is >> std::boolalpha >> b;
    return b;
}

std::string xtos(xmlChar* toString) { 
  std::string ret = (const char*)(*toString);
  return ret; 
}

std::unique_ptr<SourceAST>
dispatch_on_logic_tag(xmlNodePtr node)
{
  if(xmlStrcmp(node->name, (const xmlChar*)"if") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
      return std::make_unique<SourceAST_if_Verilog>(node);
    }
    else {
      return std::make_unique<SourceAST_if_C>(node);
    }
  }
  else if(xmlStrcmp(node->name, (const xmlChar*)"assign") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
      return std::make_unique<SourceAST_assignment_Verilog>(node);
    }
    else {
      return std::make_unique<SourceAST_assignment_C>(node);
    }
  }
  else {
    std::cerr << "Unknown xml tag in \'<logic>\' block.\n";
    exit(-1);
  }
}

std::unique_ptr<SourceAST>
parse_logic(xmlNodePtr node)
{
  xmlNodePtr child = xmlFirstElementChild(node);
  std::unique_ptr<SourceAST> result = dispatch_on_logic_tag(child);
  std::unique_ptr<SourceAST> * last_node = &result;

  while(child != NULL) {
    (*last_node)->append_next(dispatch_on_logic_tag(child));
    last_node = &(*last_node)->next;

    child = child->next;
  }

  return std::move(result);
}

