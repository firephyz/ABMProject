#include "agent_model.h"
#include "communications.h"

#include <iostream>

size_t dimensions[] = {2, 3};
AgentModel loaded_model(SpatialType::D2_Cartesian, 2, dimensions);
CommsNeighborhood neighborhood = {NeighborhoodType::Square, 1};

struct mlm_data_t {
  int id;
};

// TODO Should mlm_data space be allocated by the runtime?
void *
AgentModel::modelNewAgent() {
  static int next_id = 0x12;
  struct mlm_data_t * data = (mlm_data_t *)malloc(sizeof(mlm_data_t));
  data->id = next_id;
  next_id++;
  return data;
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
