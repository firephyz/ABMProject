#ifndef AGENT_MODEL_INCLUDED
#define AGENT_MODEL_INCLUDED

#include "communications.h"
#include "spatial.h"

// Forward declare to avoid circular dependency
typedef struct comms_neighborhood_t CommsNeighborhood;
class AgentModel;

// Must be outside class to be resolved by dlsym
// Declared and defined in main during model loading with libdl
extern void *              (*modelNewAgentPtr)(AgentModel * this_class, void * position);
extern void *              (*modelGiveAnswerPtr)(AgentModel * this_class, void * mlm_data);
extern void                (*modelReceiveAnswerPtr)(AgentModel * this_class, void * mlm_data, void * answer);
extern CommsNeighborhood&  (*modelGiveNeighborhoodPtr)(AgentModel * this_class, void * mlm_data);
extern void                (*modelUpdateAgentPtr)(AgentModel * this_class, void * mlm_data);

class AgentModel {
public:
/***********************************************************************
 * Models must specify these elements                                  *
 ***********************************************************************/
  SpatialType space_type;
  int num_dimensions;
  size_t * dimensions;

  // Model makers must implement functions below
  void * modelNewAgent(void * position);
  void * modelGiveAnswer(void * mlm_data);
  void modelReceiveAnswer(void * mlm_data, void * answer);
  CommsNeighborhood& modelGiveNeighborhood(void * mlm_data);
  void modelUpdateAgent(void * mlm_data);
/***********************************************************************
 * Model specifc elements done                                         *
 ***********************************************************************/



  AgentModel(SpatialType space_type, int num_dimensions, size_t * dimensions)
    : space_type(space_type)
    , num_dimensions(num_dimensions)
    , dimensions(dimensions)
  {}

  // So we can call these functions in runtime code nicely
  inline void * newAgent(void * position) { return (*modelNewAgentPtr)(this, position); }
  inline void * giveAnswer(void * mlm_data) { return (*modelGiveAnswerPtr)(this, mlm_data); }
  inline void receiveAnswer(void * mlm_data, void * answer) { return (*modelReceiveAnswerPtr)(this, mlm_data, answer); }
  inline CommsNeighborhood& giveNeighborhood(void * mlm_data) { return (*modelGiveNeighborhoodPtr)(this, mlm_data); }
  inline void updateAgent(void * mlm_data) { return (*modelUpdateAgentPtr)(this, mlm_data); }
};

#endif