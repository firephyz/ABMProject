#ifndef POSITION_INCLUDED
#define POSITION_INCLUDED

#include <string>
#include <vector>
#include <libxml2/libxml/parser.h>

#include "agent/dimension.h"

// Used to specify the extent to with the initial agent declaration covers
// the simulation space. Can be used to declared large regions as a certain
// kind of agent with certain values or can specify single positions.
struct AgentPosition {
  AgentPosition() = default;
  AgentPosition(std::vector<dimension> pos)
    : dimensions(pos)
    , is_region_value(false)
  {}
  AgentPosition(std::vector<int>& dims, xmlNodePtr node, const std::string& str);
  AgentPosition& operator=(const AgentPosition& other) {
    dimensions = other.dimensions;
    is_region_value = other.is_region_value;
    return *this;
  }
  bool operator!=(const AgentPosition& other) const;
  bool operator<(const AgentPosition& other) const;
  bool overlaps(const AgentPosition& other) const;
  bool is_region() const {
    return is_region_value; // pre-computed at construction time
  }
  AgentPosition begin() const;
  AgentPosition end() const;
  long to_integer() const;

  std::vector<dimension> dimensions;
  bool is_region_value = false;
};

#endif
