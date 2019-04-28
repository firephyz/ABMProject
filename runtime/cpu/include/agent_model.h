#ifndef AGENT_MODEL_INCLUDED
#define AGENT_MODEL_INCLUDED

#include "communications.h"
#include "spatial.h"
#include <cstdlib>

#include <vector>
#include <string.h>

// Forward declare to avoid circular dependency
typedef struct comms_neighborhood_t CommsNeighborhood;
class AgentModel;

// Must be outside class to be resolved by dlsym
// Declared and defined in main during model loading with libdl
extern void *              (*modelNewAgentPtr)(AgentModel * this_class, void * position, const SimCell * cell);
extern void *              (*modelGiveAnswerPtr)(AgentModel * this_class, void * mlm_data);
extern void                (*modelReceiveAnswerPtr)(AgentModel * this_class, void * mlm_data, void * answer);
extern CommsNeighborhood&  (*modelGiveNeighborhoodPtr)(AgentModel * this_class, void * mlm_data);
extern void                (*modelUpdateAgentPtr)(AgentModel * this_class, void * mlm_data);
extern void                (*modelLogPtr)(AgentModel * this_class, void * mlm_data);

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
    size_t * query_position_array = (size_t *)query_position;
    for(int i = 0; i < num_dimensions; ++i) {
      if(query_position_array[i] != position[i]) {
        return false;
      }
    }
  
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
  const SpatialType space_type;
  const int num_dimensions;
  const size_t * dimensions;

  // Model makers must implement functions below
  void * modelNewAgent(void * position, const SimCell * cell);
  void * modelGiveAnswer(void * mlm_data);
  void modelReceiveAnswer(void * mlm_data, void * answer);
  CommsNeighborhood& modelGiveNeighborhood(void * mlm_data);
  void modelUpdateAgent(void * mlm_data);
  void modelLog(void * mlm_data);
/***********************************************************************
 * Model specifc elements done                                         *
 ***********************************************************************/


  // constexpr so it is constructed in place in the library at compile time
  constexpr AgentModel(SpatialType space_type, int num_dimensions, const size_t * dimensions)
    : space_type(space_type)
    , num_dimensions(num_dimensions)
    , dimensions(dimensions)
  {}

  // So we can call these functions in runtime code nicely
  inline void * newAgent(void * position, const SimCell * cell) { return (*modelNewAgentPtr)(this, position, cell); }
  inline void * giveAnswer(void * mlm_data) { return (*modelGiveAnswerPtr)(this, mlm_data); }
  inline void receiveAnswer(void * mlm_data, void * answer) { return (*modelReceiveAnswerPtr)(this, mlm_data, answer); }
  inline CommsNeighborhood& giveNeighborhood(void * mlm_data) { return (*modelGiveNeighborhoodPtr)(this, mlm_data); }
  inline void updateAgent(void * mlm_data) { return (*modelUpdateAgentPtr)(this, mlm_data); }
  inline void Log(void * mlm_data) { return (*modelLogPtr)(this, mlm_data); }
};

#endif
