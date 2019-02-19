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
  size_t position_index;
  void * mlm_data;

  SimCell(int position_index);
};

class SimSpace< {
public:
  SpatialType space_type;
  std::vector<SimCell> cells;

  SimSpace();
  ~SimSpace();
};

#endif