#include "agent_model.h"
#include "communications.h"

#include <iostream>
#include <algorithm>
#include <array>

const int num_dimensions = 2;
const size_t dimensions[] = {3, 3};

template class SimAgent<num_dimensions>;
using InitAgent = SimAgent<num_dimensions>;
const std::array<InitAgent, 3> initial_agents {InitAgent({0, 0}), InitAgent({0, 1}), InitAgent({2, 1})};

AgentModel loaded_model(SpatialType::D2_Cartesian, num_dimensions, dimensions);
CommsNeighborhood neighborhood = {NeighborhoodType::NCube, 1};

struct mlm_data_t {
  int id;
};

// TODO Should mlm_data space be allocated by the runtime?
void *
AgentModel::modelNewAgent(void * position) {
  // Only return new agent data if one has been declared in the model
  if(std::find_if(initial_agents.begin(), initial_agents.end(),
    [&](const auto& agent){
      return agent.is_at_position(position);
  }) != initial_agents.end()) {
    static int next_id = 0x1;
    struct mlm_data_t * data = (mlm_data_t *)malloc(sizeof(mlm_data_t));
    data->id = next_id;
    next_id++;
    return data;
  }

  return NULL;
}

void *
AgentModel::modelGiveAnswer(void * mlm_data) {
  struct mlm_data_t * data = (struct mlm_data_t *)mlm_data;
  std::cout << "Agent " << data->id << ": Giving answer..." << std::endl;
  return NULL;
}

void
AgentModel::modelReceiveAnswer(void * mlm_data, void * answer) {
  struct mlm_data_t * data = (struct mlm_data_t *)mlm_data;
  //std::cout << "Agent " << data->id << ": Receiving answer..." << std::endl;
  std::cout << "Agent " << data->id << ": Receiving answer..." << std::endl;
}

CommsNeighborhood&
AgentModel::modelGiveNeighborhood(void * mlm_data) {
  struct mlm_data_t * data = (struct mlm_data_t *)mlm_data;
  std::cout << "Agent " << data->id << ": Giving neighborhood..." << std::endl;
  return neighborhood;
}

void
AgentModel::modelUpdateAgent(void * mlm_data) {
  struct mlm_data_t * data = (struct mlm_data_t *)mlm_data;
  std::cout << "Agent " << data->id << ": Updating..." << std::endl;
}
