#include <iostream>
#include <libxml2/libxml/parser.h>

#include "util.h"
#include "agent/concrete_agent.h"
#include "agent/initial_agent.h"

ConcreteInitialAgent::ConcreteInitialAgent(xmlNodePtr node)
{
  static int unique_id = 0;
  this->unique_id = unique_id;
  ++unique_id;

  xmlAttrPtr xml_attr = xmlGetAttribute(node, "location");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Initial agent declaration is missing its \'location\' attribute." << std::endl;
    exit(-1);
  }

  position = AgentPosition(node, std::string((const char *)xml_attr->children->content));

  xml_attr = xmlGetAttribute(node, "type");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Initial agent declaration is missing its \'type\' attribute." << std::endl;
    exit(-1);
  }

  agent_type = std::string((const char *)xml_attr->children->content);

  xmlNodePtr curNode = xmlFirstElementChild(node);
  while(curNode != NULL) {
    if(xmlStrcmp(curNode->name, (const xmlChar *)"var") != 0) {
      std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Initial agent declarations only accept \'var\' tags as child xml nodes." << std::endl;
      exit(-1);
    }

    vars.emplace_back(curNode);

    curNode = xmlNextElementSibling(curNode);
  }
}

// enumerates all agents that lie in the the AgentPosition
InitialAgentIterator
ConcreteInitialAgent::enumerate() const
{
  return InitialAgentIterator(*this);
}

bool
ConcreteInitialAgent::operator!=(const ConcreteInitialAgent& other) const
{
  // TODO implement != check for vars
  return agent_type != other.agent_type;
}
