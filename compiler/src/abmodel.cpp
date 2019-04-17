#include "abmodel.h"

#include <string>
#include <sstream>

std::string
ABModel::to_c_source()
{
  std::stringstream result;

  // Include headers
  result << "\u0023include \"agent_model.h\"\n";
  result << "\u0023include \"communications.h\"\n";
  result << "\n";

  // Declare spatial info
  result << "size_t dimensions[] = {2, 3};\n";

  // Declare neighborhoods

  // Construct model object

  // Declare mlm_data structure

  // Declare agent constructors

  // Declare functions for asking questions

  // Declare functions for receiving answers

  // Declare functions for giving neighborhoods

  // Declare functions for updating each agent

  return result.str();
}

void
ABModel::add_agent(AgentForm& agent)
{
  agents.push_back(std::move(agent));
}

std::string
ABModel::to_string()
{
  std::stringstream result;

  for(auto& agent : agents) {
    result << agent.to_string();
  }

  return result.str();
}