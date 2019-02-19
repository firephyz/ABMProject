#include "spatial.h"
#include "agent_model.h"

#include <vector>

// Required model info
//   - Explicit or implicit spatial data:
//       - Explicit: sim space data is outside of agents
//       - Implicit: sim space data is internal to agents. Moves processing to agent internals

SimCell::SimCell(int position_index)
  : position_index(position_index)
  , mlm_data(NULL)
{}

SimSpace::SimSpace(AgentModel& model) 
  : space_type(model.space_type)
  , sim_cells()
{
  num_cells = 1;
  for(int dim = 0; dim < model.num_dimensions; ++dim) {
    num_cells *= model.dimensions[dim];
  }

  for(int i = 0; i < num_cells; ++i) {
    sim_cells.emplace_back(i);
  }
}

SimSpace::~SimSpace() {
  if(sim_cells != NULL) delete[] sim_cells;
}