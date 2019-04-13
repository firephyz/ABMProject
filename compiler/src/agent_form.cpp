#include "agent_form.h"
#include "source_ast.h"

#include <memory>
#include <string>

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