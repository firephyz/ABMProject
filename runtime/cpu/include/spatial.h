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
  static SimSpace& space;
public:
  size_t * position;
  mlm_data * data;

  SimCell(SimSpace& space, int position_index);
  SimCell(const SimCell& other) = delete;
  SimCell(SimCell&& other) noexcept;

  bool operator!=(SimCell& other);
  bool is_empty() { return data == NULL; }
  std::string readPosition() const;
  
  size_t * get_position() const { return position; }
  size_t position_to_index(const size_t * pos) const;
  // Moves mlm_data from this cell to a different sim cell
  const size_t * move_mlm_data(size_t * new_pos);
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
