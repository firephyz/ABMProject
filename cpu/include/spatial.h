#ifndef SPATIAL_INCLUDED
#define SPATIAL_INCLUDED

#include <vector>

#include "agent_model.h"

class SimCell {
public:
  int x;
  int y;
  void * mlm_data;
};

class SimSpace {
public:
  std::vector<SimCell> cells;

  SimSpace();
  ~SimSpace();
};

#endif