#include "spatial.h"
#include "agent_model.h"

#include <vector>

extern AgentModel * loaded_model;

// Required model info
//   - Explicit or implicit spatial data:
//       - Explicit: sim space data is outside of agents
//       - Implicit: sim space data is internal to agents. Moves processing to agent internals

SimCell::SimCell(SimSpace& space, int position_index)
  : position(new size_t[space.num_dimensions])
  , mlm_data(NULL)
{
  size_t mod_amount = 1;
  for(int dim = 0; dim < space.num_dimensions; ++dim) {
    mod_amount *= space.dimensions[dim];
  }

  for(int dim = space.num_dimensions - 1; dim >= 0; --dim) {
    mod_amount /= space.dimensions[dim];
    position[dim] = position_index / mod_amount;
    position_index = position_index % mod_amount;
  }

  // create agent data if one is present. Keep NULL otherwise
  mlm_data = loaded_model->newAgent(position);
}

SimCell::SimCell(SimCell&& other) noexcept
  : position(std::move(other.position))
  , mlm_data(other.mlm_data)
{
}

bool SimCell::operator!=(SimCell& other)
{
  return this != &other;
}

SimSpace::SimSpace(AgentModel& model) 
  : space_type(model.space_type)
  , num_dimensions(model.num_dimensions)
  , dimensions(new size_t[model.num_dimensions])
  , cells()
{
  int num_cells = 1;
  for(int dim = 0; dim < model.num_dimensions; ++dim) {
    num_cells *= model.dimensions[dim];
    dimensions[dim] = model.dimensions[dim];
  }

  cells.reserve(num_cells);
  for(int i = 0; i < num_cells; ++i) {
    cells.emplace_back(*this, i);
  }
}

SimSpace::~SimSpace() {
}