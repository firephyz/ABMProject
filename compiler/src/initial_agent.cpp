#include "initial_agent.h"
#include "util.h"
#include "abmodel.h"

#include <libxml2/libxml/parser.h>
#include <string>
#include <iostream>
#include <sstream>

extern ABModel abmodel;

std::pair<AgentPosition::relation_t, int>
parse_relation(const std::string str)
{
  if(str[0] == '<') {
    int value = -1;
    auto relation = AgentPosition::relation_t::LT;
    if(str[1] == '=') {
      value = std::stoi(str.substr(2));
    }
    else {
      value = std::stoi(str.substr(1)) - 1;
    }
    return {relation, value};
  }
  else if(str[0] == '>') {
    int value = -1;
    auto relation = AgentPosition::relation_t::GT;
    if(str[1] == '=') {
      value = std::stoi(str.substr(2));
    }
    else {
      value = std::stoi(str.substr(1)) + 1;
    }
    return {relation, value};
  }
  else {
    return {AgentPosition::relation_t::None, std::stoi(str)};
  }
}

AgentPosition::dimension::dimension(const std::string& str)
{
  if(str == "all") {
    position_type = type_t::All;
    return;
  }

  auto comma_pos = str.find(",");
  if(comma_pos == std::string::npos) {
    std::pair<relation_t, int> r = parse_relation(str);
    relation = r.first;
    first_value = r.second;
    position_type = type_t::Absolute_Position;
  }
  else {
    std::string first_part = str.substr(0,comma_pos);
    std::string second_part = str.substr(comma_pos+1);
    std::pair<relation_t, int> r1 = parse_relation(first_part);
    std::pair<relation_t, int> r2 = parse_relation(second_part);

    if(r1.first == relation_t::LT) {
      if(r2.first == relation_t::LT) {
        relation = relation_t::LTLT;
      }
      else {
        relation = relation_t::LTGT;
      }
    }
    else {
      if(r2.first == relation_t::LT) {
        relation = relation_t::GTLT;
      }
      else {
        relation = relation_t::GTGT;
      }
    }

    first_value = r1.second;
    second_value = r2.second;
    position_type = type_t::Relational_Position;
  }
}

// TODO fix possible edge cases when first or second value is on a dimension boundary
bool
AgentPosition::dimension::overlaps(const AgentPosition::dimension& other) const
{
  if(is_binary()) {
    if(other.is_binary()) {
      if(relation == relation_t::LTGT || other.relation == relation_t::LTGT) {
        return true;
      }
      else {
        return (second_value >= other.first_value) || (first_value <= other.second_value);
      }
    }
    else {
      return other < *this;
    }
  }
  else {
    if(other.is_binary()) {
      if(other.relation == relation_t::LTGT) {
        return true;
      }
      else {
        if(relation == relation_t::GT) {
          return first_value <= other.second_value;
        }
        else {
          return first_value >= other.first_value;
        }
      }
    }
    else {
      if((relation == relation_t::LT && other.relation == relation_t::LT)
        || (relation == relation_t::GT && other.relation == relation_t::GT)) {
        return true;
      }
      else {
        if(relation == relation_t::GT) {
          return first_value <= other.first_value;
        }
        else {
          return first_value >= other.first_value;
        }
      }
    }
  }
}

// sorts according to initial_agent position relation type.
// So we know which initial agents to process first
bool
AgentPosition::dimension::operator<(const struct dimension& other) const
{
  switch(position_type) {
    case type_t::None:
      return false; // error
    case type_t::Absolute_Position:
      if(other.position_type == type_t::Absolute_Position) {
        if(overlaps(other)) {
          std::cerr << "Multiple initial agents declared with the same location." << std::endl;
          exit(-1);
        }
      }
      return true;
    case type_t::Relational_Position:
      switch(other.position_type) {
        case type_t::None:
          return false; // error
        case type_t::Absolute_Position:
          return false;
        case type_t::Relational_Position:
//          if(overlaps(other)) {
//            std::cerr << "Multiple initial agents declared with an overlapping location." << std::endl;
//            exit(-1);
//          }
          return true;
        case type_t::All:
          return true;
      }
    case type_t::All:
      if(other.position_type == type_t::All) {
        std::cerr << "Multiple initial agents declared with location of \'All\'." << std::endl;
        exit(-1);
      }
      return false;
  }

  return false;
}

AgentPosition::dimension&
AgentPosition::dimension::operator-(int value) {
  first_value -= value;
  return *this;
}

AgentPosition::dimension&
AgentPosition::dimension::operator+(int value) {
  first_value += value;
  return *this;
}

AgentPosition::dimension
AgentPosition::dimension::begin() const
{
  dimension result(*this);
  result.second_value = -1;
  result.position_type = dimension::type_t::Absolute_Position;
  result.relation = relation_t::None;
  switch(position_type) {
    case type_t::None:
      break;
    case type_t::Absolute_Position:
      break;
    case type_t::Relational_Position:
      switch(relation) {
        case relation_t::None:
          break;
        case relation_t::LT:
          result.first_value = 0;
          break;
        case relation_t::GT:
          result.first_value = first_value;
          break;
        case relation_t::LTLT:
          break;
        case relation_t::LTGT:
          result.first_value = 0;
          break;
        case relation_t::GTLT:
          result.first_value = first_value;
          break;
        case relation_t::GTGT:
          break;
      }
      break;
    case type_t::All:
      result.first_value = 0;
      break;
  }
  return result;
}

AgentPosition::dimension
AgentPosition::dimension::end(int dim_index) const
{
  int max_value = abmodel.init.dimension_sizes[dim_index];
  dimension result(*this);
  result.second_value = -1;
  result.position_type = dimension::type_t::Absolute_Position;
  result.relation = relation_t::None;
  switch(position_type) {
    case type_t::None:
      break;
    case type_t::Absolute_Position:
      break;
    case type_t::Relational_Position:
      switch(relation) {
        case relation_t::None:
          break;
        case relation_t::LT:
          result.first_value = first_value + 1;
          break;
        case relation_t::GT:
          result.first_value = max_value;
          break;
        case relation_t::LTLT:
          break;
        case relation_t::LTGT:
          result.first_value = max_value;
          break;
        case relation_t::GTLT:
          result.first_value = second_value + 1;
          break;
        case relation_t::GTGT:
          break;
      }
      break;
    case type_t::All:
      result.first_value = max_value;
      break;
  }
  return result;
}

AgentPosition::dimension
AgentPosition::dimension::next(const AgentPosition::dimension& region_dim) const
{
  dimension result(*this);
  result.position_type = dimension::type_t::Absolute_Position;
  result.relation = relation_t::None;
  switch(region_dim.position_type) {
    case type_t::None:
      break;
    case type_t::Absolute_Position:
      break;
    case type_t::Relational_Position:
      switch(region_dim.relation) {
        case relation_t::None:
          break;
        case relation_t::LT:
          result.first_value = first_value + 1;
          break;
        case relation_t::GT:
          result.first_value = first_value + 1;
          break;
        case relation_t::LTLT:
          break;
        case relation_t::LTGT:
          result.first_value = first_value + 1;
          if(result.first_value == region_dim.first_value + 1) {
            result.first_value = region_dim.second_value;
          }
          break;
        case relation_t::GTLT:
          result.first_value = first_value + 1;
          break;
        case relation_t::GTGT:
          break;
      }
      break;
    case type_t::All:
      result.first_value = first_value + 1;
      break;
  }
  return result;
}

bool
AgentPosition::overlaps(const AgentPosition& other) const
{
  uintptr_t dim_index = 0;
  for(auto& dim : dimensions) {
    if(!dim.overlaps(other.dimensions[dim_index])) {
      return false;
    }
    ++dim_index;
  }
  return true;
}

AgentPosition::AgentPosition(xmlNodePtr node, const std::string& str)
{
  if(str.length() == 0) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "\'location\' attribute expects a non-empty string." << std::endl;
    exit(-1);
  }

  size_t pos = 0;
  size_t next_pos = str.find(" ", 0);

  while(true) {
	if(next_pos == std::string::npos) next_pos = str.length();
    dimensions.emplace_back(str.substr(pos, next_pos));
    if(!is_region_value &&
      (dimensions.back().relation != relation_t::None ||
       dimensions.back().position_type == dimension::type_t::All)) {
        is_region_value = true;
    }

    if(next_pos == str.length()) break;
    pos = next_pos + 1;
    next_pos = str.find(" ", pos);
  }
}

AgentPosition
AgentPosition::begin() const
{
  std::vector<dimension> start_pos;
  for(auto& dim : dimensions) {
    start_pos.push_back(dim.begin());
  }
  return AgentPosition(start_pos);
}

AgentPosition
AgentPosition::end() const
{
  std::vector<dimension> end_pos;
  for(uint dim_index = 0; dim_index < dimensions.size(); ++dim_index) {
    auto& dim = dimensions[dim_index];
    if(dim_index == dimensions.size() - 1) {
      end_pos.push_back(dim.end(dim_index));
    }
    else {
      end_pos.push_back(dim.begin());
    }
  }
  return AgentPosition(end_pos);
}

VarValueOverride::VarValueOverride(xmlNodePtr node)
{
  xmlAttrPtr xml_attr = xmlGetAttribute(node, "id");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Var override tag is missing its \'id\' attribute." << std::endl;
    exit(-1);
  }

  var_id = std::string((const char *)xml_attr->children->content);

  xml_attr = xmlGetAttribute(node, "value");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Var override tag is missing its \'value\' attribute." << std::endl;
    exit(-1);
  }

  value = std::string((const char *)xml_attr->children->content);
}

// only call if you're sure the AgentPosition isn't a region
std::string
InitialAgent::gen_constructor() const
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
  result << "})";
  return result.str();
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

ConcreteInitialAgent::ConcreteInitialAgent(xmlNodePtr node)
{
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

/*********************************************************************
 * Methods for logical initial agents                                *
 *********************************************************************/

LogicalInitialAgent::LogicalInitialAgent(const ConcreteInitialAgent& agent)
  : actual(agent)
{
  position = agent.position.begin();
}

void
LogicalInitialAgent::next()
{
  for(uint dim_index = 0; dim_index < position.dimensions.size(); ++dim_index) {
    auto& dim = position.dimensions[dim_index];
    auto& region_dim = actual.position.dimensions[dim_index];
    // Let the last dimension roll over so we can encounter the region.end()
    if(dim == region_dim.end(dim_index) && dim_index != position.dimensions.size() - 1) {
      dim = region_dim.begin();
    }
    else {
      dim = dim.next(region_dim);
      break;
    }
  }
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

/************************************************************
 * Iterator for initial agents. Constructs temporary agents *
 * for use during enumeration of regions                    *
 ************************************************************/

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
