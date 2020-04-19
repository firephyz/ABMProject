#include "abmodel.h"
#include "agent/concrete_agent.h"

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

std::string
ABModel::to_c_source()
{
  std::stringstream result;

  // Include headers
  result << "\u0023include \"agent_model.h\"\n";
  result << "\u0023include \"communications.h\"\n";
  result << "\u0023include <algorithm>\n";
  result << "\u0023include <array>\n";
  result << "\u0023include <iostream>\n";
  result << "\u0023include <sstream>\n";
  result << "\n";

  // TODO split this following enum into multiple ones for each agent
  // Declare agent state enum
  result << "enum class AgentState {\n";
  result << "\tSTATE_NONE,\n";
  for(auto& agent : agents) {
    for(auto& state : agent.getStates()) {
      result << "\t" << state.gen_state_enum_name(agent.getName()) << ",\n";
    }
  }
  result << "};\n";
  result << "\n";

  // Generate structs for holding agent initial data
  for(auto& agent : agents) {
    result << "struct agent_init_" << agent.getName() << " {\n";
    for(auto& var : agent.getAgentScopeBindings()) {
      result << "\t" << var.get_type().to_string() << " " << var.gen_var_name() << ";\n";
    }
    result << "};\n\n";
  }

  for(auto& ag : init.agents) {
    result << "struct agent_init_" << ag.agent_type << " init_" << ag.unique_id;
    result << " = {";
    for(auto& var : find_agent_by_name(ag.agent_type).getAgentScopeBindings()) {
      auto init_var = std::find_if(ag.vars.begin(), ag.vars.end(),
        [&](const VarValueOverride& other)
        {
          return other.name == var.getName();
      });

      if(init_var == ag.vars.end()) {
        result << var.gen_c_default_value();
      }
      else {
        if(init_var->name == "state") {
          result << "AgentState::STATE_" << ag.agent_type << "_" << init_var->init_value;
        }
        else {
          result << init_var->init_value;
        }
      }
      result << ", ";
    }
    result << "};\n";
  }
  result << "\n";

  // Declare space for initial agents.
  // This does not fill in the initial values for each agent specified. It only notifies the runtime
  // which simulation cells need to have agents in them at the start of the simulation
  result << "constexpr int num_dimensions = " << gen_space_size() << ";\n";
  result << "template class SimAgent<num_dimensions>;\n";
  result << "using InitAgent = SimAgent<num_dimensions>;\n";
  std::vector<std::string> initial_agent_defs = gen_initial_agent_defs();
  result << "const std::array<InitAgent, " << initial_agent_defs.size() << "> initial_agents {";
  for(uint index = 0; index < initial_agent_defs.size(); index++) {
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
  result << "AgentModel loaded_model(\"" << model_name << "\", " << gen_spatial_enum() << ", num_dimensions, dimensions);\n";
  result << "\n";

  // Moved declaration into each agent specific mlm_data struct
  // // Declare neighborhoods. Must get types from agents
  // for(auto& agent : agents) {
  //   result << agent.getNeighborhood().to_c_source() << "\n";
  // }
  // result << "\n";

  // Declare enum
 	result << "enum class AgentType {\n";
	for(auto& agent : agents) {
    result << "\t" << agent.gen_enum_type_name() << ",\n";
  }
  result << "};\n\n";

  // Declare answer_block base class
  result << "struct answer_block {\n";
  result << "\tAgentType type_tag;\n\n";
  result << "\
  answer_block(AgentType type) : type_tag(type) {}\n";
  result << "};\n";
  result << "\n";

  // Declare mlm_data structure and the derived ones for each agents
  result << gen_mlm_data_structs();

	result << gen_enum_to_strings();
  // Declare agent constructors
  result << gen_new_agent_func() << "\n";
  result << "\n";

  // Declare functions for giving answers
  result << gen_give_answer_code() << "\n";
  result << "\n";

  // Declare functions for receiving answers
  result << gen_receive_answer_code();

  // Declare functions for giving neighborhoods
  result << gen_give_neighborhood_code();

  // Declare functions for updating each agent
  result << gen_update_agent_code();

  // Declare functions for logging
  result << gen_logging_funct();

  // Delcare functions related to global time tick
  result << gen_tick_code();

  return result.str();
}

std::string
ABModel::gen_give_answer_code()
{
  std::stringstream result;

  result << "\
answer_block *\n\
AgentModel::modelGiveAnswer(mlm_data * data) {\n\
  data->record_answers();\n\
  return data->give_answers();\n\
}";

  return result.str();
}

std::string
ABModel::gen_receive_answer_code()
{
  std::stringstream result;
  result << "\
void\n\
AgentModel::modelReceiveAnswer(mlm_data * data, answer_block * answer) {\n\
  data->receive_answers(answer);\n\
  data->process_questions();\n\
}\n";
  result << "\n";

  for(auto& agent : agents) {
    result << agent.gen_receive_answer_code();
  }

  return result.str();
}

std::string
ABModel::gen_give_neighborhood_code()
{
  std::stringstream result;
  result << "\
const CommsNeighborhood&\n\
AgentModel::modelGiveNeighborhood(mlm_data * data) {\n\
  return data->neighborhood;\n\
}\n";
  result << "\n";
  return result.str();
}

std::string
ABModel::gen_update_agent_code()
{
  std::stringstream result;
  result << "\
void\n\
AgentModel::modelUpdateAgent(mlm_data * data) {\n\
  data->update_agent();\n\
}\n";
  result << "\n";

  for(auto& agent : agents) {
    result << agent.gen_agent_update_code();
  }
  return result.str();
}

std::string
ABModel::gen_tick_code()
{
  std::stringstream result;
  result << "\
void\n\
AgentModel::modelTick() {\n";
  //result << "std::cout << \"Tick.. \" << std::endl;\n";
  result << "}\n";
  result << "\n";
  return result.str();
}

std::string
ABModel::gen_new_agent_func()
{
  std::stringstream result;

  // function for figuring out how much space the agent needs
  result << "\
mlm_data *\n\
allocate_agent_space(const uint type, SimCell * cell, void * init_data)\n\
{\n\
  switch(type) {\n";
  for(auto& agent : agents) {
    result << "\t\tcase " << agent_to_uint(agent) << \
      ": return new " << agent.gen_mlm_data_string() << "(cell, (struct agent_init_" << agent.getName() << " *)init_data);\n";
  }
  result << "\t\tdefault:\n\t\t\tstd::cerr << \"Runtime failure. Unknown agent uint id.\" << std::endl;\n";
  result << "\t\t\texit(-1);\n";
  result << "\t}\n";
  result << "\treturn NULL;\n";
  result << "}\n";
  result << "\n";

  // Function that creates all the new agents
  result << "\
mlm_data *\n\
AgentModel::modelNewAgent(void * position, SimCell * cell) {\n\
  auto agent_iter = std::find_if(initial_agents.begin(), initial_agents.end(),\n\
    [&](const auto& agent){\n\
      return agent.is_at_position(position);\n\
    });\n\
  if(agent_iter != initial_agents.end()) {\n\
    struct mlm_data * data = allocate_agent_space(agent_iter->getAgentType(), cell, agent_iter->getInitData());\n\
    return data;\n\
  }\n\
\n\
  return NULL;\n\
}";

  return result.str();
}

std::string
ABModel::gen_mlm_data_structs()
{
  std::stringstream result;
/*
  result <<"\
struct mlm_data {\n\
  const SimCell * sim_cell;\n\
  const AgentType type;\n\
  const CommsNeighborhood neighborhood;\n\
\n\
  mlm_data(const SimCell * sim_cell, const AgentType type, CommsNeighborhood neighborhood)\n\
    : sim_cell(sim_cell)\n\
    , type(type)\n\
    , neighborhood(neighborhood)\n\
  {}\n\
  virtual void record_answers() = 0;\n\
  virtual answer_block * give_answers() const = 0;\n\
  virtual void receive_answers(answer_block * answer) = 0;\n\
  virtual void process_questions() = 0;\n\
  virtual void update_agent() = 0;\n\
};\n" << "\n";
*/
  for(auto& agent : agents) {
    result << agent.gen_mlm_data_struct() << "\n";
  }

  return result.str();
}

std::vector<std::string>
ABModel::gen_initial_agent_defs()
{
  std::vector<std::string> result;
  for(auto& agent : init.agents) {
    //if(agent.position.is_region()) {
      for(auto& single_agent : agent.enumerate()) {
        result.push_back(single_agent.gen_constructor());
      }
    // }
    // else {
    //   result.push_back(agent.gen_constructor());
    // }
  }
  return result;
}

std::string
ABModel::gen_spatial_enum()
{
	switch(numOfDimensions) {
		case 1:
			return "SpatialType::D1_Cartesian";
    case 2:
  		return "SpatialType::D2_Cartesian";
		case 3:
			return "SpatialType::D3_Cartesian";
		default:
			return "SpatialType::D2_Cartesian";
  }
}

std::string
ABModel::gen_space_dims()
{
  std::stringstream result;
  result << "{";
  for(uint dim_index = 0; dim_index < init.dimension_sizes.size(); ++dim_index) {
    result << init.dimension_sizes[dim_index];
    if(dim_index != init.dimension_sizes.size() - 1) {
      result << ", ";
    }
  }
  result << "}";
  return result.str();
}

std::string
ABModel::gen_space_size()
{
	return std::to_string(numOfDimensions);
}

std::string
ABModel::gen_logging_funct()
{
  std::stringstream result;
  result <<"\
std::string \
AgentModel::modelLog(mlm_data * data) {\n\
	std::stringstream logStr;"\
  << std::endl;

  for (auto &agent : agents) {
  	if (agent.log_en == true) {
   		result << agent.gen_log_code() << std::endl;
    }
  }

  result << "	return logStr.str();\n}" << std::endl;
  result << "\n";
	return result.str();
}

void
ABModel::add_agent(AgentForm& agent)
{
  agents.push_back(std::move(agent));
}

uint
ABModel::agent_to_uint(const AgentForm& agent) const
{
  uint id = 0;
  for(auto& other : agents) {
    if(&other == &agent) {
      return id;
    }
    ++id;
  }

  std::cerr << "Could not locate agent \'" << agent.getName() << "\' for turning into uints in during code-gen." << std::endl;
  exit(-1);
}

uint
ABModel::agent_to_uint_by_name(const std::string& name) const
{
  return agent_to_uint(find_agent_by_name(name));
}

const AgentForm&
ABModel::find_agent_by_name(const std::string& name) const
{
  auto agent_iter = std::find_if(
    agents.begin(),
    agents.end(),
    [&](const AgentForm& agent) {
      return agent.getName() == name;
    });

  if(agent_iter == agents.end()) {
    std::cerr << "Failed to convert agent name \'" << name << "\' into uint.";
    std::cerr << " Could not find corresponding agent." << std::endl;
    exit(-1);
  }

  return *agent_iter;
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

std::string
ABModel::gen_enum_to_strings()
{
	std::stringstream result;
	// Declare enum to_string funct
	for(auto& agent : agents) {
		result << "std::string mlm_data_" << agent.agent_name <<"::AgentTypeEnumToString() {\n";
		result << "\tswitch(*(this->type)) {\n";
		result << "\t\tcase AgentType::" << agent.gen_enum_type_name() << ":\n";
		result << "\t\t\treturn \"" <<  agent.gen_enum_type_name() << "\";\n";
	}
	result << "\t\t}\n}\n\n";


  // Declare enum to_string funct
for(auto& agent : agents) {
		result << "std::string mlm_data_" << agent.agent_name << "::AgentStateEnumToString() {\n";
	  result << "\tswitch(this->state) {\n";
	for(auto& state : agent.getStates()) {
			result << "\t\tcase AgentState::" << state.gen_state_enum_name(agent.getName()) << ":\n";
			result << "\t\t\treturn \"" <<  state.gen_state_enum_name(agent.getName()) << "\";\n";
	  }
  }
	result << "\t\t}\n}\n\n";

	return result.str();
}
