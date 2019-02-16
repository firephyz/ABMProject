#ifndef SPATIAL_INCLUDED
#define SPATIAL_INCLUDED

#include <vector>

#include "agent_model.h"

class SimCell {
public:
  bool has_agent;
  AgentType agent_id;
  std::array<uint8_t> local_agent_bytes;
  std::array<uint8_t> local_parameter_bytes;
};

class SimSpace {
public:
  std::vector<SimCell> cells;
  
  SimSpace(AgentModel& model);
  ~SimSpace();
};

#endif