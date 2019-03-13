#ifndef ABMODEL_INCLUDED
#define ABMODEL_INCLUDED

#include "agent_form.h"

#include <vector>
#include <sstream>

class ABModel {
  std::vector<AgentForm> agents;
public:
  ABModel() = default;
  ABModel(const ABModel& other) = delete;
  ABModel(ABModel&& other) = default;

  std::stringstream to_c_source();
};

#endif