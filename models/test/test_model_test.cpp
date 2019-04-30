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

struct mlm_data_AgentType_Alive {
	int neighbors_alive;
};

struct mlm_data_AgentType : public mlm_data {
	AgentState state = AgentState::STATE_AgentType_Alive;
	bool is_alive = false;
	union {
		struct mlm_data_AgentType_Alive locals_Alive;
	};

	mlm_data_AgentType(const SimCell * sim_cell)
		: mlm_data(sim_cell, AgentType::AGENT_AgentType)
	{}
};



mlm_data *
allocate_agent_space(const uint type, const SimCell * cell)
{
  switch(type) {
		case 0: return new mlm_data_AgentType(cell);
		default:
			std::cerr << "Runtime failure. Unknown agent uint id." << std::endl;
			exit(-1);
	}
	return NULL;
}

void *
AgentModel::modelNewAgent(void * position, const SimCell * cell) {
  auto agent_iter = std::find_if(initial_agents.begin(), initial_agents.end(),
    [&](const auto& agent){
      return agent.is_at_position(position);
    });
  if(agent_iter != initial_agents.end()) {
    struct mlm_data * data = allocate_agent_space(agent_iter->getAgentType(), cell);
    return data;
}

void AgentModel::modelReceiveAnswer(void * mlm_data, void * answer) {
  struct mlm_data* data = (struct mlm_data*)mlm_data;
  //std::cout << "Agent " << data->id << ": Receiving answer..." << std::endl;
  std::cout << "Agent: Receiving answer..." << std::endl;
}

CommsNeighborhood&
AgentModel::modelGiveNeighborhood(mlm_data  * mlm_data) {
  struct mlm_data * data = (struct mlm_data *)mlm_data;
  std::cout << "Agent: Giving neighborhood..." << std::endl;
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

