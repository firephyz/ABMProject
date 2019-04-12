#ifndef AGENT_FORM_INCLUDED
#define AGENT_FORM_INCLUDED

#include "compiler_types.h"
#include "source_ast.h"

#include <vector>
#include <string>
#include <memory>

class StateInstance {
  const std::string state_name;
  std::vector<SymbolBinding> state_scope_vars;
  std::unique_ptr<SourceAST> source;
public:
  StateInstance(const std::string& name);
  StateInstance(const StateInstance&) = delete;
  StateInstance(StateInstance&&) = default;

  std::vector<SymbolBinding>& getStateScopeBindings();

  // moves argument
  void add_logic(std::unique_ptr<SourceAST>&& source);
};

class AgentForm {
  const std::string agent_name;
  std::vector<SymbolBinding> agent_scope_vars;
  std::vector<StateInstance> states;
public:
  AgentForm(const std::string& name);
  AgentForm(const AgentForm&) = delete;
  AgentForm(AgentForm&&) = default;

  std::vector<SymbolBinding>& getAgentScopeBindings();
 
  // moves arguments out info AgentForm
  StateInstance& add_state(StateInstance&& state);

  // Generates a list of symbol bindings representing the bindings available from
  // the agent scope and from the state scope.
  ContextBindings genContextBindings(StateInstance& state);
};

#endif
