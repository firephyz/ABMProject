#ifndef AGENT_MODEL_INCLUDED
#define AGENT_MODEL_INCLUDED

#include "communications.h"
#include "spatial.h"
#include <cstdlib>

#include <vector>
#include <string.h>
#include <iostream>
#include <sstream>

// Forward declare to avoid circular dependency
typedef struct comms_neighborhood_t CommsNeighborhood;
class AgentModel;
class SimCell;
enum class AgentType;

// currently produced during compiler code-gen
// struct mlm_data;
struct answer_block;

// moved mlm_data struct here for BOIDS
struct mlm_data {
  SimCell * sim_cell;
  AgentType * type;
  const CommsNeighborhood neighborhood;
  mlm_data(SimCell * sim_cell, const AgentType aType, CommsNeighborhood neighborhood)
    : sim_cell(sim_cell)
    , type((AgentType *)malloc(sizeof(AgentType)))
    , neighborhood(neighborhood)
  {
		*type = aType;
	}
  virtual void record_answers() = 0;
  virtual answer_block * give_answers() const = 0;
  virtual void receive_answers(answer_block * answer) = 0;
  virtual void process_questions() = 0;
};

// Must be outside class to be resolved by dlsym
// Declared and defined in main during model loading with libdl
extern mlm_data *          (*modelNewAgentPtr)(AgentModel * this_class, void * position, const SimCell * cell);
extern answer_block *      (*modelGiveAnswerPtr)(AgentModel * this_class, mlm_data * data);
extern void                (*modelReceiveAnswerPtr)(AgentModel * this_class, mlm_data * data, answer_block * answer);
extern const CommsNeighborhood&  (*modelGiveNeighborhoodPtr)(AgentModel * this_class, mlm_data * data);
extern void                (*modelUpdateAgentPtr)(AgentModel * this_class, mlm_data * data);
extern std::string         (*modelLogPtr)(AgentModel * this_class, mlm_data * data);
extern void                (*modelTickPtr)(AgentModel * this_class);
/**********************************************************
 * The following class is only constructed in the MLM cpp files.
 * Not intended to be used by runtime
 **********************************************************/
// holds data for initial agents in the simulation
// template and constexpr strangeness is so we can construct
// arrays of initial agents in the MLM binary with no runtime overhead
template<int N_DIM>
class SimAgent {
public:
  static const int num_dimensions = N_DIM;

  constexpr SimAgent(const uint agent_type, const std::initializer_list<size_t>& position)
    : agent_type(agent_type)
  {
    std::copy(position.begin(), position.end(), this->position);
  }

  bool is_at_position(void * query_position) const
  {
    std::stringstream position_str;
    size_t * query_position_array = (size_t *)query_position;
    for(int i = 0; i < num_dimensions; ++i) {
      if(query_position_array[i] != position[i]) {
        return false;
      }
      position_str << query_position_array[i];
      if(i != num_dimensions - 1) {
        position_str << ", ";
      }
    }

    std::cout << "Making Agent: " << position_str.str() << std::endl;
  
    return true;
  }

  uint getAgentType() const { return agent_type; }

private:
  uint agent_type;
  size_t position[num_dimensions];
};

class AgentModel {
public:
/***********************************************************************
 * Models must specify these elements                                  *
 ***********************************************************************/ 
  // For Logging
  const char * model_name;
  const SpatialType space_type;
  const int num_dimensions;
  const size_t * dimensions;
  
  // Model makers must implement functions below
  mlm_data *          modelNewAgent(void * position, SimCell * cell);
  answer_block *      modelGiveAnswer(mlm_data * data);
  void                modelReceiveAnswer(mlm_data * data, answer_block * answer);
  const CommsNeighborhood&  modelGiveNeighborhood(mlm_data * data);
  void                modelUpdateAgent(mlm_data * data);
  std::string         modelLog(mlm_data * data);
  void                modelTick();
/***********************************************************************
 * Model specifc elements done                                         *
 ***********************************************************************/


  // constexpr so it is constructed in place in the library at compile time
  constexpr AgentModel(const char * model_name, SpatialType space_type, int num_dimensions, const size_t * dimensions)
    : model_name(model_name)
    , space_type(space_type)
    , num_dimensions(num_dimensions)
    , dimensions(dimensions)
  {}

  // So we can call these functions in runtime code nicely
  inline mlm_data * newAgent(void * position, const SimCell * cell) { return (*modelNewAgentPtr)(this, position, cell); }
  inline answer_block * giveAnswer(mlm_data * data) { return (*modelGiveAnswerPtr)(this, data); }
  inline void receiveAnswer(mlm_data * data, answer_block * answer) { return (*modelReceiveAnswerPtr)(this, data, answer); }
  inline const CommsNeighborhood& giveNeighborhood(mlm_data * data) { return (*modelGiveNeighborhoodPtr)(this, data); }
  inline void updateAgent(mlm_data * data) { return (*modelUpdateAgentPtr)(this, data); }
  inline std::string Log(mlm_data * data) { return (*modelLogPtr)(this, data); }
  inline void Tick() { return (*modelTickPtr)(this); }
};

#endif
