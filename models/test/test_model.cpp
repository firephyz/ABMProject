#include "agent_model.h"
#include "communications.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>

constexpr int num_dimensions = 2;
template class SimAgent<num_dimensions>;
using InitAgent = SimAgent<num_dimensions>;
const std::array<InitAgent, 4> initial_agents {InitAgent(0, {4, 0}), InitAgent(0, {4, 1}), InitAgent(0, {4, 3}), InitAgent(0, {4, 4})};

size_t dimensions[] = {5, 5};
AgentModel loaded_model("GameOfLife", SpatialType::D2_Cartesian, num_dimensions, dimensions);

enum class AgentType {
	AGENT_AgentType,
};

enum class AgentState {
	STATE_NONE,
	STATE_AgentType_Alive,
};

struct answer_block {
	AgentType type_tag;

  answer_block(AgentType type) : type_tag(type) {}
};

struct answer_AgentType : public answer_block {
	bool AgentType_living_neighbors = false;

	answer_AgentType(AgentType type) : answer_block(type) {}
};

struct mlm_data {
  const SimCell * sim_cell;
  const AgentType type;
  const CommsNeighborhood neighborhood;

  mlm_data(const SimCell * sim_cell, const AgentType type, CommsNeighborhood neighborhood)
    : sim_cell(sim_cell)
    , type(type)
    , neighborhood(neighborhood)
  {}
  virtual void record_answers() = 0;
  virtual answer_block * get_answers() const = 0;
};

struct mlm_data_AgentType_Alive {
	int neighbors_alive;
};

struct mlm_data_AgentType : public mlm_data {
	static struct answer_AgentType answers;
	AgentState state = AgentState::STATE_AgentType_Alive;
	bool is_alive = false;
	union {
		struct mlm_data_AgentType_Alive locals_Alive;
	};

	mlm_data_AgentType(const SimCell * sim_cell)
		: mlm_data(sim_cell, AgentType::AGENT_AgentType, (CommsNeighborhood)(CommsNeighborhood){NeighborhoodType::NCube, 2})
	{
		answers = answer_AgentType(AgentType::AGENT_AgentType);
	}
  void record_answers() {
		answers.AgentType_living_neighbors = is_alive;
	}
  answer_block * get_answers() const { return (answer_block *)&answers; }
};
struct answer_AgentType mlm_data_AgentType::answers(AgentType::AGENT_AgentType);

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

mlm_data *
AgentModel::modelNewAgent(void * position, const SimCell * cell) {
  auto agent_iter = std::find_if(initial_agents.begin(), initial_agents.end(),
    [&](const auto& agent){
      return agent.is_at_position(position);
    });
  if(agent_iter != initial_agents.end()) {
    struct mlm_data * data = allocate_agent_space(agent_iter->getAgentType(), cell);
    return data;
  }

  return NULL;
}

answer_block *
AgentModel::modelGiveAnswer(mlm_data * data) {
  std::cout << "Agent: Giving answer..." << std::endl;
  data->record_answers();
  return data->get_answers();
}

void
AgentModel::modelReceiveAnswer(mlm_data * data, answer_block * answer) {
  std::cout << "Agent: Receiving answer..." << std::endl;
}

const CommsNeighborhood&
AgentModel::modelGiveNeighborhood(mlm_data * data) {
  std::cout << "Agent: Giving neighborhood..." << std::endl;
  return data->neighborhood;
}

void
AgentModel::modelUpdateAgent(mlm_data * data) {
  std::cout << "Agent: Updating..." << std::endl;
}

std::string AgentModel::modelLog(mlm_data * data) {
	std::stringstream logStr;
	if (data->type == AgentType::AGENT_AgentType) {
		logStr << ":" << "\\" << data->sim_cell->readPosition();
	}
	return logStr.str();
}

void
AgentModel::modelTick() {
  std::cout << "Tick.. " << std::endl;
}


