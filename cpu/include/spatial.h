#ifndef SPATIAL_INCLUDED
#define SPATIAL_INCLUDED

#include <vector>

#include "agent_model.h"

typedef enum {
  D1_Cartesian,
  D2_Cartesian,
  D3_Cartesian
} SpatialType;

class SimCell {
public:
  size_t * position;
  void * mlm_data;

  SimCell(int position_index);
};

class SimSpace {
public:
  SpatialType space_type;
  int num_dimensions;
  size_t * dimensions;
  std::vector<SimCell> cells;

  SimSpace(AgentModel& model);
  ~SimSpace();
};

#endif