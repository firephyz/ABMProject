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

using namespace std;
#include <libxml2/libxml/parser.h>
#include <string>

#include <iomanip>
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
    if(xmlStrcmp(curNode->name, (const xmlChar *)"CommsInterface") != 0) {
      std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Agent is missing its \'CommsInterface\' tag." << std::endl;
      exit(-1);
    }
    auto xml_attr = xmlGetAttribute(curNode, "neighborhood");
    if(xml_attr == NULL) {
      std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Comms interface does not have a neighborhood type." << std::endl;
      exit(-1);
    }
    // TODO do something with neighborhood type

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
      return std::make_unique<SourceAST_return_Verilog>(ctxt, node);
    }
    else {
      return std::make_unique<SourceAST_return_C>(ctxt, node);
    }
  }
  else if(xmlStrcmp(node->name, (const xmlChar*)"response") == 0) {
    if(pargs.target == OutputTarget::FPGA) {
      return std::make_unique<SourceAST_response_Verilog>(ctxt, node);
    }
    else {
      return std::make_unique<SourceAST_response_C>(ctxt, node);
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

