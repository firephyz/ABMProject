#ifndef ABMODEL_INCLUDED
#define ABMODEL_INCLUDED

#include "agent_form.h"
#include "compiler_types.h"
#include "initial_agent.h"

#include <vector>
#include <string>

struct ABM_Environment {
  ContextBindings genEnvironmentContext();
};

struct InitialState {
  std::vector<int> dimension_sizes;
  std::vector<ConcreteInitialAgent> agents;
};

struct ABModel {
  ABM_Environment environment;
  InitialState init;
  std::vector<AgentForm> agents;

  ABModel() = default;
  ABModel(const ABModel& other) = delete;
  ABModel(ABModel&& other) = default;

  // for code gen  
  std::string to_c_source();
  std::vector<std::string> gen_initial_agent_defs();
  std::string gen_spatial_enum();
  std::string gen_space_dims();
  std::string gen_space_size();
  std::string gen_mlm_data_struct();

  void add_agent(AgentForm& agent);
  size_t num_agents() const { return agents.size(); }
  AgentForm& get_agent_by_index(const size_t index) { return agents[index]; }

  std::string to_string();
};

#endif
