#include "abmodel.h"

#include <sstream>

std::stringstream
ABModel::to_c_source()
{
  std::stringstream result;

  return result;
}

void
ABModel::add_agent(AgentForm& agent)
{
  agents.push_back(std::move(agent));
}