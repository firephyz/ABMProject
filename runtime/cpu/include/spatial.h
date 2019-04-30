#ifndef SPATIAL_INCLUDED
#define SPATIAL_INCLUDED

#include <vector>
#include <cstddef>
#include <iostream>

typedef enum {
  D1_Cartesian,
  D2_Cartesian,
  D3_Cartesian,
} SpatialType;

#include "agent_model.h"

// Forward declaration
class SimSpace;
struct mlm_data;

class SimCell {
public:
  size_t * position;
  mlm_data * data;

  SimCell(SimSpace& space, int position_index);
  SimCell(const SimCell& other) = delete;
  SimCell(SimCell&& other) noexcept;

  bool operator!=(SimCell& other);
  bool is_empty() { return data == NULL; }
  std::string readPosition() const;

};

// Forward declare
class AgentModel;

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
