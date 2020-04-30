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
	virtual std::string AgentTypeEnumToString() = 0;
	virtual std::string AgentStateEnumToString() = 0;
  virtual void update_agent() = 0;
};

// Must be outside class to be resolved by dlsym
// Declared and defined in main during model loading with libdl
extern mlm_data * (*modelNewAgentPtr)(AgentModel * this_class, SimCell * cell);
extern answer_block * (*modelGiveAnswerPtr)(AgentModel * this_class, mlm_data * data);
extern void (*modelReceiveAnswerPtr)(AgentModel * this_class, mlm_data * data, answer_block * answer);
extern const CommsNeighborhood&  (*modelGiveNeighborhoodPtr)(AgentModel * this_class, mlm_data * data);
extern void (*modelUpdateAgentPtr)(AgentModel * this_class, mlm_data * data);
extern void (*modelTickPtr)(AgentModel * this_class);

extern std::string (*modelLogPtr)(AgentModel * this_class, mlm_data * data);
extern std::string (*modelAgentTypeEnumToStringPtr)(AgentModel * this_class, AgentType type);

/**********************************************************
 * The following class is only constructed in the MLM cpp files.
 * Not intended to be used by runtime
 **********************************************************/
// holds data for initial agents in the simulation
// template
class InitAgent {
public:
  InitAgent() = default;
  InitAgent(std::string init);

  uint getAgentType() const { return agent_type; }
  size_t getInitIndex() const { return init_data_index; }

private:
  uint agent_type;
  size_t init_data_index;
};

class AgentModel {
public:
/***********************************************************************
 * Models must specify these elements                                  *
 ***********************************************************************/
  // Model makers must implement functions below
  mlm_data *          modelNewAgent(SimCell * cell);
  answer_block *      modelGiveAnswer(mlm_data * data);
  void                modelReceiveAnswer(mlm_data * data, answer_block * answer);
  const CommsNeighborhood&  modelGiveNeighborhood(mlm_data * data);
  void                modelUpdateAgent(mlm_data * data);
  std::string         modelLog(mlm_data * data);
  void                modelTick();
/***********************************************************************
 * Model specifc elements done                                         *
 ***********************************************************************/

  // For Logging
  const char * model_name;
  const SpatialType space_type;
  const int num_dims;
  const size_t * dimensions;
  const char * init_path;

  // malloc space after loading model, vectors are not constexpr
  size_t initial_agents_size;
  InitAgent * initial_agents;

  // constexpr so it is constructed in place in the library at compile time
  constexpr AgentModel(const char * model_name,
                       SpatialType space_type,
                       int num_dimensions,
                       const size_t * dimensions,
                       const char * init_path)
    : model_name(model_name)
    , space_type(space_type)
    , num_dims(num_dimensions)
    , dimensions(dimensions)
    , init_path(init_path)
    , initial_agents_size(0)
    , initial_agents(NULL)
  {}

  void modelLoadInitState(const char * init_filename);

  // So we can call these functions in runtime code nicely
  inline mlm_data * newAgent(SimCell * cell) { return (*modelNewAgentPtr)(this, cell); }
  inline answer_block * giveAnswer(mlm_data * data) { return (*modelGiveAnswerPtr)(this, data); }
  inline void receiveAnswer(mlm_data * data, answer_block * answer) { return (*modelReceiveAnswerPtr)(this, data, answer); }
  inline const CommsNeighborhood& giveNeighborhood(mlm_data * data) { return (*modelGiveNeighborhoodPtr)(this, data); }
  inline void updateAgent(mlm_data * data) { return (*modelUpdateAgentPtr)(this, data); }
  inline std::string Log(mlm_data * data) { return (*modelLogPtr)(this, data); }
	inline void Tick() { return (*modelTickPtr)(this); }
};

#endif
