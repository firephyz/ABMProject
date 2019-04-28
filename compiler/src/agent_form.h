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

  const std::vector<SymbolBinding>& getStateScopeBindings() const;
  std::vector<SymbolBinding>& getStateScopeBindingsMut() { return state_scope_vars; }

  // moves argument
  void add_logic(std::unique_ptr<SourceAST>&& source);

  std::string gen_state_enum_name(const std::string& str) const;
  const std::string& getName() const { return state_name; }

  std::string to_string();
};

// forward declare
struct CommsNeighborhood;

class AgentForm {
  const std::string agent_name;
  std::vector<SymbolBinding> agent_scope_vars;
  std::vector<StateInstance> states;
  std::vector<std::shared_ptr<Question>> questions;
  std::unique_ptr<CommsNeighborhood> neighborhood;

public:
  AgentForm(const std::string& name);
  AgentForm(const AgentForm&) = delete;
  AgentForm(AgentForm&&) = default;

  const std::vector<SymbolBinding>& getAgentScopeBindings() const;
  std::vector<SymbolBinding>& getAgentScopeBindingsMut() { return agent_scope_vars; }
  std::vector<StateInstance>& getStates() { return states; }
  std::vector<std::shared_ptr<Question>>& getQuestions() { return questions; }
  const std::vector<StateInstance>& getStates() const { return states; }
  const std::string& getName() const { return agent_name; }

  StateInstance& add_state(StateInstance&& state);
  void set_neighborhood(std::unique_ptr<CommsNeighborhood>&& n);

  // Generates a list of symbol bindings representing the bindings available from
  // the agent scope and from the state scope.
  ContextBindings genContextBindings();
  ContextBindings genContextBindings(StateInstance& state);
  const CommsNeighborhood& getNeighborhood() const { return *(neighborhood.get()); }

  std::string gen_enum_type_name() const;
  std::string gen_mlm_data_struct() const;
  std::string gen_mlm_data_string() const;

  std::string to_string();
};

// Include after so comms.h gets the AgentForm declaration
#include "comms.h"

#endif
