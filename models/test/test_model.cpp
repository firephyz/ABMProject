#include "agent_model.h"
#include "communications.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <array>

const int num_dimensions = 2;
const size_t dimensions[] = {3, 3};

template class SimAgent<num_dimensions>;
using InitAgent = SimAgent<num_dimensions>;
const std::array<InitAgent, 3> initial_agents {InitAgent(0, {0, 0}), InitAgent(0, {0, 1}), InitAgent(0, {2, 1})};

AgentModel loaded_model("DEFAULT_NAME", SpatialType::D2_Cartesian, num_dimensions, dimensions);
CommsNeighborhood neighborhood = {NeighborhoodType::NCube, 1};


enum class AgentType {
	AGENT_AgentType,
};

enum class AgentState {
	STATE_NONE,
	STATE_AgentType_Alive,
};

struct mlm_data {
  const SimCell * sim_cell;
  const AgentType type;
  mlm_data(const SimCell * sim_cell, const AgentType type)
    : sim_cell(sim_cell)
    , type(type)
  {}
};


// TODO Should mlm_data space be allocated by the runtime?
mlm_data *
AgentModel::modelNewAgent(void * position, const SimCell * sim_cell) {
  // Only return new agent data if one has been declared in the model
  if(std::find_if(initial_agents.begin(), initial_agents.end(),
    [&](const auto& agent){
      return agent.is_at_position(position);
  }) != initial_agents.end()) {
    // static int next_id = 0x1;
   mlm_data* data = (mlm_data*)malloc(sizeof(mlm_data)); 
   return data;
  }

  return NULL;
}

void *
AgentModel::modelGiveAnswer(mlm_data * mlm_data) {
  std::cout << "Agent Giving answer..." << std::endl;
  return NULL;
}

void AgentModel::modelReceiveAnswer(void * mlm_data, void * answer) {
  struct mlm_data* data = (struct mlm_data*)mlm_data;
  //std::cout << "Agent " << data->id << ": Receiving answer..." << std::endl;
  std::cout << "Agent: Receiving answer..." << std::endl;
}

CommsNeighborhood&
AgentModel::modelGiveNeighborhood(mlm_data * mlm_data) {
  struct mlm_data_t * data = (struct mlm_data_t *)mlm_data;
  std::cout << "Agent " << data->id << ": Giving neighborhood..." << std::endl;
  return neighborhood;
}

void
AgentModel::modelUpdateAgent(mlm_data * mlm_data) {
  struct mlm_data * data = (struct mlm_data *)mlm_data;
  std::cout << "Agent: Updating..." << std::endl;
}

std::string
AgentModel::modelLog(mlm_data * mlm_data) {
  struct mlm_data * data = (struct mlm_data*)mlm_data;
  std::stringstream logStr;
  logStr << ":" << data->sim_cell->readPosition();
  return logStr.str();
}

void 
AgentModel::modelTick() {
	std::cout << "Tick.. " << std::endl;
}
