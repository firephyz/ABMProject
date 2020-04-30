#include "abmodel.h"
#include "agent/concrete_agent.h"
#include "agent/agent_iterator.h"
#include "agent/logical_init_agent.h"

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <fstream>

extern ABModel abmodel;

void
InitialState::resolveInitAgentLinks()
{
  for(auto& ag : agents) {
    const AgentForm& agent_form = abmodel.find_agent_by_name(ag.getName());
    ag.agent = &agent_form;
  }
}

void
InitialState::sortInitAgents()
{
  // sort initial_agents by type vector
  std::sort(abmodel.init.agents_by_type.begin(),
            abmodel.init.agents_by_type.end(),
            [](auto& a, auto& b) {
    return abmodel.init.agents[a].type_unique_id < abmodel.init.agents[b].type_unique_id;
  });

  // sort initial agents by position vector
  std::sort(abmodel.init.agents_by_position.begin(),
            abmodel.init.agents_by_position.end(),
            [](auto& a, auto& b) {
    return abmodel.init.agents[a].position < abmodel.init.agents[b].position;
  });
  // sort by largest to smallest so more specific init decls get
  // processed last
  std::reverse(abmodel.init.agents_by_position.begin(),
               abmodel.init.agents_by_position.end());
}

std::string
ABModel::to_c_source(long gen_unique_id)
{
  std::stringstream result;

  // Include headers
  result << "\u0023include \"agent_model.h\"\n";
  result << "\u0023include \"communications.h\"\n";
  result << "\u0023include <algorithm>\n";
  result << "\u0023include <vector>\n";
  result << "\u0023include <iostream>\n";
  result << "\u0023include <sstream>\n";
  result << "\u0023include <array>\n";
  result << "\n\u0023define UNIQUE_ID " << gen_unique_id << "\n";
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

  // Generate structs defs for holding agent initial data
  for(auto& agent : agents) {
    result << "struct agent_init_" << agent.getName() << " {\n";
    for(auto& var : agent.getAgentScopeBindings()) {
      result << "\t" << var.get_type().to_string() << " " << var.gen_var_name() << ";\n";
    }
    result << "};\n\n";
  }

  // Generate initial data array for different initial states
  for(auto& ag : agents) {
    result << gen_init_array(ag);
  }

  // Declare space for initial agents.
  // This does not fill in the initial values for each agent specified. It only notifies the runtime
  // which simulation cells need to have agents in them at the start of the simulation
  result << "constexpr int num_dimensions = " << gen_space_size() << ";\n";

  // Declare spatial info and model
  result << "size_t dimensions[] = " << gen_space_dims() << ";\n";
  result << "AgentModel loaded_model(\""
    << model_name << "\", " << gen_spatial_enum()
    << ", num_dimensions, dimensions, \"init-" << gen_unique_id << "\");\n";
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

void
ABModel::write_init_file(long unique_id)
{
  std::string filename = std::string("init-").append(std::to_string(unique_id));
  std::ofstream fileout(filename);

  // calculate size of simulation space
  int space_size = 1;
  for(auto& width : dimension_sizes) {
    space_size *= width;
  }

  // fill temporary simulation space with enumerated agents
  // more specific position decls will get processed last
  std::vector<LogicalInitialAgent> ag_space;
  ag_space.assign(space_size, LogicalInitialAgent());
  for(auto& ag_index : init.agents_by_position) {
    auto& agent = init.agents[ag_index];
    for(auto& single_agent : agent.enumerate()) {
      ag_space[single_agent.position.to_integer()] = LogicalInitialAgent(single_agent);
    }
  }

  // output init data
  fileout << space_size << std::endl;
  for(auto& ag : ag_space) {
    fileout << ag.gen_init_data() << std::endl;
  }
}

std::string
ABModel::gen_init_array(const AgentForm& ag)
{
  std::stringstream result;

  // Find where the agent vector is partited along different agent types
  std::vector<size_t> partitions = {0};
  // Iterate through agent type ids
  for(size_t ag_type_index = 0; ag_type_index < agents.size(); ++ag_type_index) {
    auto part = std::find_if(init.agents_by_type.begin(),
                             init.agents_by_type.end(),
                             [&](auto ag_index) {
      // look for next type to find boundary
      return init.agents[ag_index].type_unique_id == ag_type_index + 1;
    });
    size_t index = part - init.agents_by_type.begin();
    partitions.push_back(index);
  }

  // For each partition along agent type, generate an array of possible
  // initial states.
  auto start_index = partitions.begin();
  auto end_index = partitions.begin() + 1;
  for(; start_index != partitions.end() - 1; ++start_index,++end_index) {
    // get agent for naming the array
    auto& concrete_agent = init.agents[init.agents_by_type[*start_index]];
    result << "std::array<struct agent_init_" << concrete_agent.agent_type << ", " << init.agents.size() << "> ";
    result << "init_states_" << concrete_agent.agent_type << " = {\n";

    // iterate through agents of this type to enumerate possible init states
    for(auto ag_index = *start_index; ag_index != *end_index; ++ag_index) {
      auto& agent_of_type = init.agents[ag_index];
      result << "\t(struct agent_init_" << concrete_agent.agent_type << "){ ";

      // Generate an initial value for each variable in scope
      for(auto& symbol : agent_of_type.getAgentScopeBindings()) {
        // get var value override for concrete agent if present
        auto init_var = std::find_if(agent_of_type.vars.begin(), agent_of_type.vars.end(),
        [&](const VarValueOverride& other) {
          return other.name == symbol.getName();
        });

        if(init_var == agent_of_type.vars.end()) {
          result << symbol.gen_c_default_value();
        }
        else {
          if(init_var->name == "state") {
            result << "AgentState::STATE_" << agent_of_type.agent_type << "_" << init_var->init_value;
          }
          else {
            result << init_var->init_value;
          }
        }


        result << ", ";
      }
      result.seekp(result.tellp() - (std::streamoff)2);
      result << " },\n";
    }
    result.seekp(result.tellp() - (std::streamoff)2);
    result << "\n";
  }
  result << "};\n\n";

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
  return;\n\
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
  return;\n\
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
  result << "return;\n";
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
allocate_agent_space(const uint type, SimCell * cell, size_t init_index)\n\
{\n\
  switch(type) {\n";
  for(auto& agent : agents) {
    result << "\t\tcase " << agent_to_uint(agent) << \
      ": return new " << agent.gen_mlm_data_string() << "(cell, &init_states_" << agent.getName() << "[init_index]);\n";
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
AgentModel::modelNewAgent(SimCell * cell) {\n\
  auto& init_agent = initial_agents[cell->position_to_index()];\n\
  struct mlm_data * data = allocate_agent_space(init_agent.getAgentType(), cell, init_agent.getInitIndex());\n\
  return data;\n\
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
  for(uint dim_index = 0; dim_index < dimension_sizes.size(); ++dim_index) {
    result << dimension_sizes[dim_index];
    if(dim_index != dimension_sizes.size() - 1) {
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

size_t
ABModel::get_agent_type_index(const std::string& agent_type) const
{
  auto iter = std::find_if(agents.begin(),
                           agents.end(),
                           [&](auto& ag) {
    return agent_type == ag.getName();
  });

  if(iter == agents.end()) {
    std::cerr << " Could not find agent type \'" << agent_type << "\'." << std::endl;
    exit(-1);
  }

  return iter - agents.begin();
}

const AgentForm&
ABModel::find_agent_by_name(const std::string& name) const
{
  auto iter = std::find_if(agents.begin(), agents.end(), [&](auto& other) {
    return other.getName() == name;
  });

  if (iter == agents.end()) {
    std::cerr << "Could not find agent form with name \'" << name << "\'." << std::endl;
    exit(-1);
  }

  return *iter;
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
