#include "agent_form.h"
#include "source_ast.h"
#include "comms.h"

#include <memory>
#include <string>
#include <sstream>

StateInstance::StateInstance(const std::string& name)
  : state_name(name)
{}

std::vector<SymbolBinding>&
StateInstance::getStateScopeBindings()
{
  return state_scope_vars;
}

void
StateInstance::add_logic(std::unique_ptr<SourceAST>&& source)
{
  this->source = std::move(source);
}

std::string
StateInstance::to_string()
{
  std::stringstream result;

  result << "\tState name: " << state_name << std::endl;
  result << "\t\t\t------ Vars ------" << std::endl;
  for(auto& binding : state_scope_vars) {
    result << "\t\t\t\t" << binding.to_string();
  }
  result << "\t\t\t------ Source -------" << std::endl;
  SourceAST::set_start_depth(4);
  result << source->print_tree();

  return result.str();
}

std::string
StateInstance::gen_state_enum_name(const std::string& agent_name) const
{
  std::stringstream result;
  result << "STATE_" << agent_name << "_" << state_name;
  return result.str();
}

AgentForm::AgentForm(const std::string& name)
  : agent_name(name)
  , neighborhood(nullptr)
{}

void
AgentForm::set_neighborhood(std::unique_ptr<CommsNeighborhood>&& n)
{
  neighborhood = std::move(n);
}

std::vector<SymbolBinding>&
AgentForm::getAgentScopeBindings()
{
  return agent_scope_vars;
}

StateInstance&
AgentForm::add_state(StateInstance&& state)
{
  states.push_back(std::move(state));
  return states[states.size() - 1];
}

ContextBindings
AgentForm::genContextBindings(StateInstance& state)
{
  ContextBindings result;
  result.frames.push_back(&state.getStateScopeBindings());
  result.frames.push_back(&agent_scope_vars);
  return result;
}

ContextBindings
AgentForm::genContextBindings()
{
  ContextBindings result;
  result.frames.push_back(&agent_scope_vars);
  return result;
}

std::string
AgentForm::to_string()
{
  std::stringstream result;

  result << "Name: " << agent_name << std::endl;
  result << "\t------ Vars ------" << std::endl;
  for(auto& binding : agent_scope_vars) {
    result << "\t\t" << binding.to_string();
  }
  result << "\t------ States ------" << std::endl;
  for(auto& state : states) {
    result << "\t" << state.to_string() << std::endl;
  }
  result << "\t------ Questions -------" << std::endl;
  for(auto& question : questions) {
    result << "\t" << question->to_string() << std::endl;
  }

  // reset SourceAST printer
  SourceAST::set_start_depth(0);

  return result.str();
}

std::string
AgentForm::gen_enum_type_name() const
{
  return std::string("AGENT_") + agent_name;
}

std::string
AgentForm::gen_mlm_data_struct() const
{
  std::stringstream result;
  result << "\
struct mlm_data_" << agent_name << " : public mlm_data {\n";
  for(auto& variable : agent_scope_vars) {
    result << "\t" << variable.gen_declaration() << "\n";
  }
  // TODO place state variable union
  result << "}\n";
  return result.str();
}