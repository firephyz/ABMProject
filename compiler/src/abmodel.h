#ifndef ABMODEL_INCLUDED
#define ABMODEL_INCLUDED

#include "agent_form.h"

#include <vector>
#include <sstream>

struct EnvironmentParameters {
  int test; // fill in real values
};

class ABModel {
  EnvironmentParameters environment;
  std::vector<AgentForm> agents;
public:
  ABModel() = default;
  ABModel(const ABModel& other) = delete;
  ABModel(ABModel&& other) = default;

  std::stringstream to_c_source();
  void add_agent(AgentForm& agent);
};

#endif