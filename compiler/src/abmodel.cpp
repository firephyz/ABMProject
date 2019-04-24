#include "abmodel.h"
#include <string>
#include <sstream>

std::stringstream
ABModel::to_c_source()
{
  std::stringstream result;

  return result;
}

void ABModel::add_agent(AgentForm& agent)
{
  agents.push_back(std::move(agent));
}

void ABModel::add_to_econtext(int frameSelect, char * name, char * value, char * type, bool cnst) // check on symbolbinding definition
{
	
	SymbolBinding temp = SymbolBinding(std::string(name), strToEnum(std::string(type)), std::string(value) , cnst);
	this->environment.env.frames.at(frameSelect)->push_back(temp);
}

ContextBindings ABModel::get_env_context()
{
	return this->environment.env;
}

/*void ABModel::add_to_econtext(int frameSelect, std::string& name, struct VariableType type, void * initial_value, bool is_constant) {
	SymbolBinding temp = SymbolBinding(name, type, initial_value, is_constant);
	ABModel.Environment[frameSelect].push_back(temp);
}
*/
ContextBindings EnvironmentParameters::genEnvironmentContext() {
	
	 // frame 0 = local, frame 1 = global
	this->env = ContextBindings(2);
}

