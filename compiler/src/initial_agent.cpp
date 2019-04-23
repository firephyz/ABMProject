#include "initial_agent.h"
#include "util.h"

#include <libxml2/libxml/parser.h>
#include <string>
#include <iostream>

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
  auto comma_pos = str.find(",");
  if(comma_pos == std::string::npos) {
    std::pair<relation_t, int> r = parse_relation(str);
    relation = r.first;
    first_value = r.second;
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
      return false;
    case type_t::Relational_Position:
      switch(other.position_type) {
        case type_t::None:
          return false; // error
        case type_t::Absolute_Position:
          return false;
        case type_t::Relational_Position:
          if(overlaps(other)) {
            std::cerr << "Multiple initial agents declared with an overlapping location." << std::endl;
            exit(-1);
          }
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

bool
AgentPosition::overlaps(const AgentPosition& other) const
{
  uintptr_t dim_index = 0;
  for(auto& dim : position) {
    if(!dim.overlaps(other.position[dim_index])) {
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
  do {
    position.emplace_back(str.substr(pos, next_pos));
    pos = next_pos;
    next_pos = str.find(" ", pos);
  } while(pos != std::string::npos);
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

InitialAgent::InitialAgent(xmlNodePtr node)
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
