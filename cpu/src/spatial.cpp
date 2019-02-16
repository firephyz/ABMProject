#include "spatial.h"
#include "agent_model.h"

#include <vector>

// Required model info
//   - Explicit or implicit spatial data:
//       - Explicit: sim space data is outside of agents
//       - Implicit: sim space data is internal to agents. Moves processing to agent internals

SimCell::SimCell(AgentModel& model)
  : has_agent(false)
  , agent_id(AgentType::None)
  , local_agent_bytes(model.max_agent_size)
  , local_parameter_bytes(model.parameter_field_size)
{}

SimSpace::SimSpace(AgentModel& model) 
  : sim_cells(model.x_res * model.y_res, SimCell(model))
{
    // Set initial state from model
}

SimSpace::~SimSpace() {
    if(sim_cells != NULL) delete[] sim_cells;
}