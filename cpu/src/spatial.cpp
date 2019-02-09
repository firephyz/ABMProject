#include "spatial.h"
#include "agent_model.h"

// Required model info
//   - Explicit or implicit spatial data:
//       - Explicit: sim space data is outside of agents
//       - Implicit: sim space data is internal to agents. Moves processing to agent internals

SimSpace::SimSpace(AgentModel& model) 
  : sim_cells(NULL)
{
    if(model.isSpaceExplicit()) {
        sim_cells = new SimCell[model.x_res * model.y_res];
        // Set initial state from model
    }
    else {

    }
}

SimSpace::~SimSpace() {
    if(sim_cells != NULL) delete[] sim_cells;
}