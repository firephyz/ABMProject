#include <iostream>
#include <libxml2/libxml/parser.h>

#include "util.h"
#include "abmodel.h"
#include "agent/concrete_agent.h"
#include "agent/logical_init_agent.h"

extern ABModel abmodel;

ConcreteInitialAgent::ConcreteInitialAgent()
{}

ConcreteInitialAgent::ConcreteInitialAgent(xmlNodePtr node)
  : type_region_index(0)
  , vars()
{
  xmlAttrPtr xml_attr = xmlGetAttribute(node, "location");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Initial agent declaration is missing its \'location\' attribute." << std::endl;
    exit(-1);
  }

  this->position = AgentPosition(abmodel.dimension_sizes,
                                 node,
                                 std::string((const char *)xml_attr->children->content));

  xml_attr = xmlGetAttribute(node, "type");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Initial agent declaration is missing its \'type\' attribute." << std::endl;
    exit(-1);
  }

  this->agent_type = std::string((const char *)xml_attr->children->content);
  this->type_unique_id = abmodel.get_agent_type_index(agent_type);

  xmlNodePtr curNode = xmlFirstElementChild(node);
  while(curNode != NULL) {
    if(xmlStrcmp(curNode->name, (const xmlChar *)"var") != 0) {
      std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Initial agent declarations only accept \'var\' tags as child xml nodes." << std::endl;
      exit(-1);
    }

    this->vars.emplace_back(curNode);

    curNode = xmlNextElementSibling(curNode);
  }
}

ConcreteInitialAgent::ConcreteInitialAgent(const ConcreteInitialAgent& ag)
  : InitialAgent(static_cast<InitialAgent>(ag))
  , type_region_index(ag.type_region_index)
  , vars(ag.vars)
{}

// enumerates all agents that lie in the the AgentPosition
InitialAgentIterator
ConcreteInitialAgent::enumerate() const
{
  return InitialAgentIterator(*this);
}

/******************************************************************************
 * Override default variable values with this class
 ******************************************************************************/
VarValueOverride::VarValueOverride(xmlNodePtr node)
{
  xmlAttrPtr xml_attr = xmlGetAttribute(node, "id");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Var override tag is missing its \'id\' attribute." << std::endl;
    exit(-1);
  }

  name = std::string((const char *)xml_attr->children->content);

  xml_attr = xmlGetAttribute(node, "value");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Var override tag is missing its \'value\' attribute." << std::endl;
    exit(-1);
  }

  init_value = std::string((const char *)xml_attr->children->content);
}
