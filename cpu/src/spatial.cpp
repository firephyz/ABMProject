#include "spatial.h"
#include "agent_model.h"

#include <vector>

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
}

SimSpace::SimSpace(AgentModel& model) 
  : space_type(model.space_type)
  , num_dimensions(model.num_dimensions)
  , dimensions(new size_t[model.num_dimensions])
  , sim_cells()
{
  num_cells = 1;
  for(int dim = 0; dim < model.num_dimensions; ++dim) {
    num_cells *= model.dimensions[dim];
    dimensions[dim] = model.dimensions;
  }

  for(int i = 0; i < num_cells; ++i) {
    sim_cells.emplace_back(*this, i);
  }
}

SimSpace::~SimSpace() {
  if(sim_cells != NULL) delete[] sim_cells;
}