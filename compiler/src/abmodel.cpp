#include "abmodel.h"

#include <string>
#include <sstream>
#include <vector>

std::string
ABModel::to_c_source()
{
  std::stringstream result;

  // Include headers
  result << "\u0023include \"agent_model.h\"\n";
  result << "\u0023include \"communications.h\"\n";
  result << "\u0023include <algorithm>\n";
  result << "\u0023include <array>\n";
  result << "\n";

  // Declare space for initial agents.
  // This does not fill in the initial values for each agent specified. It only notifies the runtime
  // which simulation cells need to have agents in them at the start of the simulation
  result << "template class SimAgent<num_dimensions>;\n";
  result << "using InitAgent = SimAgent<num_dimensions>;\n";
  std::vector<std::string> initial_agent_defs = gen_initial_agent_defs();
  result << "const std::array<InitAgent, " << initial_agent_defs.size() << "> initial_agents {";
  for(uint index = 0; index < initial_agent_defs.size(), ++index) {
    auto & agent = initial_agent_defs[index];
    result << agent;
    if(index != initial_agent_defs.size() - 1) {
      result << ", ";
    }
  }
  result << "};\n";
  result << "\n";

  // Declare spatial info and model
  result << "size_t dimensions[] = " << gen_space_dims() << ";\n";
  result << "AgentModel loaded_model(" << gen_spatial_enum() << ", " << gen_space_size() << ", dimensions);\n";

  // Declare neighborhoods. Must get types from agents
  for(auto& agent : agents) {
    result << agent.neighborhood_to_c_source() << "\n";
  }
  result << "\n";

  // Declare mlm_data structure
  result << create_mlm_data_struct() << "\n";
  result << "\n";

  // Declare agent constructors
  //result << "void * AgentModel::modelNewAgent"

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