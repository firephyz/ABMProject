/*
 * GAMP PARSER 
 * 3.9.19
 */
#include "parser.h"
#include "abmodel.h"

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

ABModel& parse_model(const char * xml_model_path){
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
        if (xmlStrcmp(child->name, (const xmlChar*)"Enviroment") == 0){ 
          parseEnviroment(child);
        } else if(xmlStrcmp(child->name, (const xmlChar*)"AgentDefinitions") == 0){ 
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
				wrap = stobool((const char*)xmlGetAttribute(curNode, "wrap")->children->content);	
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
    curNode = nextElementSibling(curNode); 
  }    
  
}

void newAgentDef(xmlNodePtr agent) {
  AgentForm toAdd;
  xmlNodePtr curNode = NULL;
  std::string curAttr = "";  

  curNode = xmlFirstElementChild(agent);
  
  // Check that the first tag is Agent and go ahead and grab the name
  if (xmlStrcmp(curNode->name, (const char*)"Agent") == 0) { 
    toAdd->name = xmlGetAttribute(curNode, "type");      
  } else {
    std::cerr << "Improper Agent Definition: Missing Agent Tag" << std::endl;
  }

  std::cout << toAdd->name << std::endl;
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
