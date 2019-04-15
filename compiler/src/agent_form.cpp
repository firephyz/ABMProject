#include "agent_form.h"
#include "source_ast.h"

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

AgentForm::AgentForm(const std::string& name)
  : agent_name(name)
{}

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
    result << "\t" << question.to_string() << std::endl;
  }

  // reset SourceAST printer
  SourceAST::set_start_depth(0);

  return result.str();
}