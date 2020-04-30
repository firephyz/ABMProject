#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <libxml2/libxml/parser.h>

#include "agent/position.h"

AgentPosition::AgentPosition(std::vector<int>& dims, xmlNodePtr node, const std::string& str)
{
  if(str.length() == 0) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "\'location\' attribute expects a non-empty string." << std::endl;
    exit(-1);
  }

  size_t pos = 0;
  size_t next_pos = str.find(" ", 0);

  size_t dim_index = 0;
  while(true) {
    if(next_pos == std::string::npos) next_pos = str.length();

    dimensions.emplace_back(dims[dim_index], str.substr(pos, next_pos));
    if(!is_region_value &&
       (dimensions.back().relation != PositionRelation::None ||
        dimensions.back().position_type == dimension::type_t::All)) {
        is_region_value = true;
    }

    if(next_pos == str.length()) {
      if (dim_index + 1 != dims.size()) {
        std::cerr << "Init agent \'location\' attribute does not specify all \'"
          << dims.size() << "\' dimensions." << std::endl;
        exit(-1);
      }
      break;
    }

    pos = next_pos + 1;
    next_pos = str.find(" ", pos);
    dim_index++;
  }

  if (dimensions.size() != dims.size()) {
    std::cerr << "Init agent \'location\' attribute contains too many dimensions." << std::endl;
    exit(-1);
  }
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
  uint max_dim = dimensions.size() - 1;
  for(uint dim_index = 0; dim_index < max_dim; ++dim_index) {
    end_pos.push_back(dimensions[dim_index].begin());
  }
  end_pos.push_back(dimensions[max_dim].end(max_dim));
  return AgentPosition(end_pos);
}

long
AgentPosition::to_integer() const
{
  if (is_region_value) {
    std::cerr << "Invalid use of \'to_integer\' on a position region. Must be an absolute position." << std::endl;
    exit(-1);
  }

  long scale = 1;
  long value = 0;

  // process reversed so later dimensions have less weight
  for(auto& dim : dimensions) {
    value += scale * dim.first_value;
    scale *= dim.width;
  }

  return value;
}

bool
AgentPosition::operator!=(const AgentPosition& other) const {
  for(uint dim_index = 0; dim_index < dimensions.size(); ++dim_index) {
    if(dimensions[dim_index] != other.dimensions[dim_index]) return true;
  }
  return false;
}

bool
AgentPosition::operator<(const AgentPosition& other) const {
  for(auto other_iter = other.dimensions.begin(); other_iter != other.dimensions.end(); ++other_iter) {
      for(auto& dim : dimensions) {
        if(!(dim < *other_iter)) return false;
      }
  }
  return true;
}
