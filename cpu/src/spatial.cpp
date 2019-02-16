#include "spatial.h"
#include "agent_model.h"

#include <vector>

// Required model info
//   - Explicit or implicit spatial data:
//       - Explicit: sim space data is outside of agents
//       - Implicit: sim space data is internal to agents. Moves processing to agent internals

SimCell::SimCell()
  : mlm_data(NULL)
{}

SimSpace::SimSpace() 
  : sim_cells()
{
    for()
}

SimSpace::~SimSpace() {
    if(sim_cells != NULL) delete[] sim_cells;
}