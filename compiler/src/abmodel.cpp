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
  result << "AgentModel loaded_model(" << gen_spatial_enum() << ", " << gen_space_size() << ", dimensions);\n";

  // Declare neighborhoods. Must get types from agents
  for(auto& agent : agents) {
    result << agent.getNeighborhood().to_c_source() << "\n";
  }
  result << "\n";

  // Declare mlm_data structure
  //result << create_mlm_data_struct() << "\n";
  //result << "\n";

  // Declare agent constructors
  //result << "void * AgentModel::modelNewAgent"

  // Declare functions for asking questions

  // Declare functions for receiving answers

  // Declare functions for giving neighborhoods

  // Declare functions for updating each agent

  return result.str();
}

std::vector<std::string>
ABModel::gen_initial_agent_defs()
{
  std::vector<std::string> result;
  for(auto& agent : init.agents) {
    if(agent.position.is_region()) {
      for(auto& single_agent : agent.enumerate()) {
        result.push_back(single_agent.gen_constructor());
      }
    }
    else {
      result.push_back(agent.gen_constructor());
    }
  }
  return result;
}

std::string
ABModel::gen_spatial_enum()
{
  return std::string();
}

std::string
ABModel::gen_space_dims()
{
  return std::string();
}

std::string
ABModel::gen_space_size()
{
  return std::string();
}

void
ABModel::add_agent(AgentForm& agent)
{
  agents.push_back(std::move(agent));
}

void ABModel::add_to_econtext(int frameSelect, char * name, char * value, char * type, bool cnst) // check on symbolbinding definition
{
	VariableType TYP;
	TYP.type = strToEnum(std::string((const char*)type));
	SymbolBinding temp = SymbolBinding(std::string(name), TYP, std::string(value) , cnst);
	this->environment.env.frames.at(frameSelect)->push_back(temp);
}

ContextBindings ABModel::get_env_context()
{
	return this->environment.env;
}
/*
void ABModel::add_LRAST(std::unique_ptr<SourceAST> tempLR)
{
	this->environment.LR_AST.push_back(tempLR);
}

void ABModel::add_GRAST(std::unique_ptr<SourceAST> tempGR)
{
	this->environment.GR_AST.push_back(tempGR);
}

std::vector<std::unique_ptr<SourceAST>> ABModel::get_LRAST()
{
	return this->environment.LR_AST;
}

std::vector<std::unique_ptr<SourceAST>> ABModel::get_GRAST()
{
	return this->environment.GR_AST;
}
*/
/*void ABModel::add_to_econtext(int frameSelect, std::string& name, struct VariableType type, void * initial_value, bool is_constant) {
	SymbolBinding temp = SymbolBinding(name, type, initial_value, is_constant);
	ABModel.Environment[frameSelect].push_back(temp);
}
*/
ContextBindings EnvironmentParameters::genEnvironmentContext() {
	
	 // frame 0 = local data, frame 1 = global data
	this->env = ContextBindings(2, "ENV");
	
}

bool ABModel::wrapState()
{
	return this->environment.wrap;
}
int ABModel::dimCount()
{
	return this->environment.dims;
}

void ABModel::setDim(int D) 
{
	this->environment.dims = D;
}

void ABModel::setWrap(bool W) 
{
	this->environment.wrap = W;
}
//}

std::string
ABModel::to_string()
{
  std::stringstream result;

  for(auto& agent : agents) {
    result << agent.to_string();
  }

  return result.str();
}
