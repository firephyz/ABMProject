#ifndef AGENT_FORM_INCLUDED
#define AGENT_FORM_INCLUDED

#include "compiler_types.h"
#include "source_ast.h"

#include <vector>
#include <string>
#include <memory>

class Question;
class SourceAST;
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
  const std::unique_ptr<SourceAST>& getSource() const { return source; }

  std::string to_string();
};

// forward declare
struct CommsNeighborhood;
class Answer;
class AgentForm {
  const std::string agent_name;
  std::vector<SymbolBinding> agent_scope_vars;
  std::vector<StateInstance> states;
  std::vector<std::shared_ptr<Question>> questions;
  std::vector<std::shared_ptr<Answer>> answers;
  std::unique_ptr<CommsNeighborhood> neighborhood;

public:
  bool log_en;
  AgentForm(const std::string& name);
  AgentForm(const AgentForm&) = delete;
  AgentForm(AgentForm&&) = default;

  const std::vector<SymbolBinding>& getAgentScopeBindings() const;
  std::vector<SymbolBinding>& getAgentScopeBindingsMut() { return agent_scope_vars; }
  std::vector<StateInstance>& getStates() { return states; }
  const std::vector<StateInstance>& getStates() const { return states; }
  const std::string& getName() const { return agent_name; }

  const std::vector<std::shared_ptr<Question>>& getQuestions() const { return questions; }
  std::vector<std::shared_ptr<Question>>& getQuestionsMut() { return questions; }
  const std::vector<std::shared_ptr<Answer>>& getAnswers() const { return answers; }
  std::vector<std::shared_ptr<Answer>>& getAnswersMut() { return answers; }

  StateInstance& add_state(StateInstance&& state);
  void set_neighborhood(std::unique_ptr<CommsNeighborhood>&& n);
  void resolve_ask_links();

  // Generates a list of symbol bindings representing the bindings available from
  // the agent scope and from the state scope.
  ContextBindings genContextBindings();
  ContextBindings genContextBindings(StateInstance& state);
  const CommsNeighborhood& getNeighborhood() const { return *(neighborhood.get()); }

  std::string gen_enum_type_name() const;
  std::string gen_mlm_data_struct() const;
  std::string gen_mlm_data_string() const;
  std::string gen_log_code() const;
  std::string gen_answer_struct() const;
  std::string gen_receive_answer_code() const;
  std::string gen_responses_struct() const;
  std::string gen_reset_question_locals_code() const;
  std::string gen_agent_update_code() const;
 
  std::string to_string();
};

// Include after so comms.h gets the AgentForm declaration
#include "comms.h"

#endif
