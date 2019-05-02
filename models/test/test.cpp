#include "agent_model.h"
#include "communications.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>

constexpr int num_dimensions = 2;
template class SimAgent<num_dimensions>;
using InitAgent = SimAgent<num_dimensions>;
const std::array<InitAgent, 5> initial_agents {InitAgent(0, {4, 0}), InitAgent(0, {4, 1}), InitAgent(0, {4, 2}), InitAgent(0, {4, 3}), InitAgent(0, {4, 4})};

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

struct responses_AgentType {
	bool living_neighbors;
};

struct mlm_data_AgentType_questions {
	struct living_neighbors_t {
		int count;
	} living_neighbors;
};

struct mlm_data_AgentType_Alive {
	int neighbors_alive;
};

struct mlm_data_AgentType : public mlm_data {
	static struct answer_AgentType answers;
	static struct responses_AgentType responses;
	struct mlm_data_AgentType_questions q_locals;
	AgentState state = AgentState::STATE_AgentType_Alive;
	bool is_alive = false;
	union {
		struct mlm_data_AgentType_Alive locals_Alive;
	};

	mlm_data_AgentType(SimCell * sim_cell)
		: mlm_data(sim_cell, AgentType::AGENT_AgentType, (CommsNeighborhood)(CommsNeighborhood){NeighborhoodType::NCube, 2})
	{
		answers = answer_AgentType(AgentType::AGENT_AgentType);
	}
  void record_answers() {
		answers.AgentType_living_neighbors = is_alive;
	}
  answer_block * give_answers() const { return (answer_block *)&answers; }
  void receive_answers(answer_block * answer);
  void process_questions();
	std::string AgentTypeEnumToString();
	std::string AgentStateEnumToString();
	int process_question_living_neighbors(mlm_data_AgentType_questions::living_neighbors_t * locals)
;};
struct answer_AgentType mlm_data_AgentType::answers(AgentType::AGENT_AgentType);
struct responses_AgentType mlm_data_AgentType::responses;

std::string mlm_data_AgentType::AgentTypeEnumToString() {
	switch(*(this->type)) {
		case AgentType::AGENT_AgentType:
			return "AGENT_AgentType";
		}
}

std::string mlm_data_AgentType::AgentStateEnumToString() {
	switch(this->state) {
		case AgentState::STATE_AgentType_Alive:
			return "STATE_AgentType_Alive";
		}
}

mlm_data *
allocate_agent_space(const uint type, SimCell * cell)
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
AgentModel::modelNewAgent(void * position, SimCell * cell) {
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
  return data->give_answers();
}

void
AgentModel::modelReceiveAnswer(mlm_data * data, answer_block * answer) {
  data->receive_answers(answer);
  data->process_questions();
  std::cout << "Agent: Receiving answer..." << std::endl;
}

void
mlm_data_AgentType::receive_answers(answer_block * answers)
{
	switch(answers->type_tag) {
		case AgentType::AGENT_AgentType: {
			answer_AgentType * agent_answers = static_cast<answer_AgentType *>(answers);
			responses.living_neighbors = agent_answers->AgentType_living_neighbors;
			break;
		}
	}
}

void
mlm_data_AgentType::process_questions()
{
	process_question_living_neighbors(&q_locals.living_neighbors);
}

int
mlm_data_AgentType::process_question_living_neighbors(mlm_data_AgentType_questions::living_neighbors_t * locals)
{
	if (responses.living_neighbors == true) {
		locals->count = (locals->count + 1);
	}
	locals_Alive.neighbors_alive = locals->count;
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
if (*(data->type) == AgentType::AGENT_AgentType) {
logStr << ":" << data->AgentTypeEnumToString() << "%" << data->AgentStateEnumToString() << "\\" << data->sim_cell->readPosition();
}
	return logStr.str();
}

void
AgentModel::modelTick() {
  std::cout << "Tick.. " << std::endl;
}


