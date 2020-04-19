#include "agent/initial_agent.h"
#include "util.h"
#include "abmodel.h"

#include <libxml2/libxml/parser.h>
#include <string>
#include <iostream>
#include <sstream>

extern ABModel abmodel;

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

bool
InitialAgent::base_neq(const InitialAgent& a, const InitialAgent& other) {
  for(uint dim_index = 0; dim_index < a.position.dimensions.size(); ++dim_index) {
    if(a.position.dimensions[dim_index] != other.position.dimensions[dim_index]) return true;
  }
  return false;
}

bool InitialAgent::operator<(const InitialAgent& other) const {
  auto other_iter = other.position.dimensions.begin();
  for(auto& dim : position.dimensions) {
    if(dim > *other_iter) return false;
  }
  return true;
}

/******************************************************************************
 * Iterator for initial agents. Constructs temporary agents                   *
 * for use during enumeration of regions                                      *
 ******************************************************************************/

InitialAgentIterator&
InitialAgentIterator::begin()
{
  agent.position = agent.actual.position.begin();
  return *this;
}

InitialAgentIterator&
InitialAgentIterator::end()
{
  agent.position = agent.actual.position.end();
  return *this;
}

bool
InitialAgentIterator::operator!=(const InitialAgentIterator& other) const
{
  return agent != other.agent;
}

InitialAgentIterator&
InitialAgentIterator::operator++()
{
  agent.next();
  return *this;
}

LogicalInitialAgent&
InitialAgentIterator::operator*()
{
  return agent;
}

/******************************************************************************
 * Methods for logical initial agents                                         *
 ******************************************************************************/

LogicalInitialAgent::LogicalInitialAgent(const ConcreteInitialAgent& agent)
  : actual(agent)
{
  position = agent.position.begin();
}

void
LogicalInitialAgent::next()
{
  for(uint dim_index = 0; dim_index < position.dimensions.size(); ++dim_index) {
    const dimension& region_dim = actual.position.dimensions[dim_index];
    auto dim = position.dimensions[dim_index].next(region_dim);
    // Let the last dimension roll over so we can encounter the region.end()
    if(dim == region_dim.end(dim_index) && dim_index != position.dimensions.size() - 1) {
      this->position.dimensions[dim_index] = region_dim.begin();
    }
    else {
      this->position.dimensions[dim_index] = dim;
      break;
    }
  }
}

// only call if you're sure the AgentPosition isn't a region
std::string
LogicalInitialAgent::gen_constructor() const
{
  std::stringstream result;
  result << "InitAgent(" << abmodel.agent_to_uint_by_name(getAgentName()) << ", {";

  uint index = 0;
  for(auto& dim : position.dimensions) {
    result << dim.first_value;
    if(index != position.dimensions.size() - 1) {
      result << ", ";
    }
    index++;
  }
  result << "}, &init_" << actual.unique_id << ")";
  return result.str();
}

bool
LogicalInitialAgent::operator!=(const LogicalInitialAgent& other) const {
  if(actual != other.actual) return true;
  if(InitialAgent::base_neq(*this,other)) return true;
  return false;
}

const std::string&
LogicalInitialAgent::getAgentName() const
{
  return actual.getAgentName();
}
