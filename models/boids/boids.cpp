#include "agent_model.h"
#include "communications.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <cmath>

constexpr int num_dimensions = 2;
template class SimAgent<num_dimensions>;
using InitAgent = SimAgent<num_dimensions>;
const std::array<InitAgent, 5> initial_agents {InitAgent(0, {4, 0}), InitAgent(0, {4, 1}), InitAgent(0, {4, 2}), InitAgent(0, {4, 3}), InitAgent(0, {4, 4})};

size_t dimensions[] = {5, 5};
AgentModel loaded_model("Boid", SpatialType::D2_Cartesian, num_dimensions, dimensions);

enum class AgentType {
	AGENT_boid,
};

enum class AgentState {
	STATE_boid_None
		
};

struct answer_block {
	AgentType type_tag;

  answer_block(AgentType type) : type_tag(type) {}
};

struct answer_boid : public answer_block {
	size_t * my_position;
	int my_alignment;
	double my_velocity;
	double my_heading;
	answer_boid(AgentType type) : answer_block(type) {}
};

struct responses_boid {
	size_t* their_position;
	int their_alignment;
	double their_velocity;
	double their_heading;
};

struct local_question_var {
	int neighbors_pos = 0;
	int neighbors_alignment = 0;
	int neighbors_head = 0;
	double pos_average;
	double alignment_average;
	double head_average;
};

struct mlm_data_boid_None {
	
};

struct mlm_data_boid : public mlm_data {
	static struct answer_boid answers;
	static struct responses_boid responses;
	struct local_question_var local_var;
	AgentState state = AgentState::STATE_boid_None;
	struct mlm_data_boid_None locals_None;
	double my_heading = 0;
	int my_alignment = 0;
	double my_velocity = 0.0;
  const int id;
	const size_t * my_position;

	mlm_data_boid(SimCell * sim_cell, int id)
		: mlm_data(sim_cell, AgentType::AGENT_boid, (CommsNeighborhood){NeighborhoodType::NCube, 5})
    , id(id)
	{
		answers = answer_boid(AgentType::AGENT_boid);
		my_position = sim_cell->get_position();
	}
  void record_answers() {
		answers.my_position = sim_cell->get_position();
		answers.my_alignment = my_alignment;
		answers.my_heading = my_heading;
		answers.my_velocity = my_velocity;
	}
  answer_block * give_answers() const { return (answer_block *)&answers; }
  void receive_answers(answer_block * answer);
  void process_questions();
	int process_question_position();
	int process_question_alignment();
	int process_question_heading();
  void update_agent();
};
struct answer_boid mlm_data_boid::answers(AgentType::AGENT_boid);
struct responses_boid mlm_data_boid::responses;

mlm_data *
allocate_agent_space(const uint type, SimCell * cell)
{
  static int next_id = 0;
  switch(type) {
		case 0: return new mlm_data_boid(cell, next_id);
		default:
			std::cerr << "Runtime failure. Unknown agent uint id." << std::endl;
			exit(-1);
	}
  next_id++;
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
  data->record_answers();
  return data->give_answers();
}

void
AgentModel::modelReceiveAnswer(mlm_data * data, answer_block * answer) {
  data->receive_answers(answer);
  data->process_questions();
}

void
mlm_data_boid::receive_answers(answer_block * answers)
{
	switch(answers->type_tag) {
		case AgentType::AGENT_boid: {
			answer_boid * agent_answers = static_cast<answer_boid *>(answers);
			responses.their_position = agent_answers->my_position;
			responses.their_alignment = agent_answers-> my_alignment;
			responses.their_heading = agent_answers->my_heading;
			responses.their_velocity = agent_answers->my_velocity;
			break;
		}
	}
}

void
mlm_data_boid::process_questions()
{
	process_question_position();
	process_question_alignment();
	process_question_heading();
}

int
mlm_data_boid::process_question_position()
{	
	size_t* fucker = sim_cell->get_position();
	double x_diff1 = responses.their_position[0];
	double x_diff2 = fucker[0];
	double x_difference = x_diff1 - x_diff2;
	double x_diff_squared = pow(x_difference, 2);
	double y_diff1 = responses.their_position[1];
	double y_diff2 = fucker[1];
	double y_difference = y_diff1 - y_diff2;
	double y_diff_squared = pow(y_difference, 2.0);
	double sum = x_diff_squared + y_diff_squared;
	double distance = sqrt(sum);
	local_var.pos_average *= local_var.neighbors_pos;
	local_var.neighbors_pos++;
	local_var.pos_average += distance;
	local_var.pos_average /= local_var.neighbors_pos;
	std::cout << "Agent " << id << " (Distance): " << distance << std::endl;
}

int
mlm_data_boid::process_question_alignment()
{
	local_var.alignment_average *= local_var.neighbors_alignment;
	local_var.neighbors_alignment++;
	local_var.alignment_average += responses.their_alignment;
	local_var.alignment_average /= local_var.neighbors_alignment;
}

int
mlm_data_boid::process_question_heading()
{
	local_var.head_average *= local_var.neighbors_head;
	local_var.neighbors_head++;
	local_var.head_average += responses.their_heading;
	local_var.head_average /= local_var.neighbors_head;
}


const CommsNeighborhood&
AgentModel::modelGiveNeighborhood(mlm_data * data) {
  return data->neighborhood;
}

void
AgentModel::modelUpdateAgent(mlm_data * data) {
  data->update_agent();
}

void
mlm_data_boid::update_agent()
{
  size_t* next_position = new size_t[2];
  next_position[0] = local_var.pos_average*cos(local_var.head_average);
  std::cout << next_position;
  next_position[1] = local_var.pos_average*sin(local_var.head_average);
  this->my_heading = local_var.head_average;
  this->my_alignment = local_var.alignment_average;
  bool result = sim_cell->move_mlm_data(next_position);
  while(!result) {
    next_position[0] = 0;
    next_position[1] = 0;
    result = sim_cell->move_mlm_data(next_position);
  }
  my_position = sim_cell->get_position();

  local_var.pos_average = 0;
  local_var.head_average = 0;
  local_var.alignment_average = 0;
  local_var.neighbors_alignment = 0;
  local_var.neighbors_head = 0;
  local_var.neighbors_pos = 0;

}

std::string AgentModel::modelLog(mlm_data * data) {
	std::stringstream logStr;
	if (*data->type == AgentType::AGENT_boid) {
		logStr << ":" << "\\" << data->sim_cell->readPosition();
	}
	return logStr.str();
}

void
reset_locals_boid()
{

}

void
AgentModel::modelTick() {
  std::cout << "Tick.. " << std::endl;
	reset_locals_boid();
}


