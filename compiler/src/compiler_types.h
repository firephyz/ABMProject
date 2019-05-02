#ifndef COMPILER_TYPES_INCLUDED
#define COMPILER_TYPES_INCLUDED

#include <string>
#include <vector>
#include <libxml2/libxml/parser.h>
#include <memory>

enum class VarTypeEnum {
  Bool,
  Integer,
  Real, // fixed-point on FPGA
  State,
};

 VarTypeEnum strToEnum(std::string& str);

// Additional type info required on FPGA targets
struct FPGATypeInfo {
  int bit_size;
  int dpoint_index; // fixed-point decimal index location. Indexed starting from LSB
};

struct VariableType {
  VarTypeEnum type;
  struct FPGATypeInfo fpga_info;
	bool log_en;


  VariableType()
    : type(VarTypeEnum::Bool)
    , fpga_info((struct FPGATypeInfo){-1, -1})
    , log_en((false))
  {}

  std::string to_string() const {
    switch(type) {
      case VarTypeEnum::Bool:
        return std::string("bool");
      case VarTypeEnum::Integer:
        return std::string("int");
      case VarTypeEnum::Real:
        return std::string("real");
      case VarTypeEnum::State:
        return std::string("AgentState");
    }

    return std::string();
  };

  std::string to_c_source() const {
    switch(type) {
      case VarTypeEnum::Bool:
        return std::string("bool");
      case VarTypeEnum::Integer:
        return std::string("int");
      case VarTypeEnum::Real:
        return std::string("double");
      case VarTypeEnum::State:
        return std::string("AgentState");
    }

    return std::string();
  }
};

enum class SymbolBindingScope {
  None,
  StateLocal,
  AgentLocal,
  Question,
  Answer,
};

class AgentForm;
class StateInstance;
class SymbolBinding {
  std::string name;
  struct VariableType type;
  std::string initial_value;
  bool is_constant;
  SymbolBindingScope scope;
  // If binding is in state scope, this will contain a pointer to that state
  const StateInstance * state = nullptr;
public:
  SymbolBinding(std::string& name, struct VariableType type, std::string& initial_value, bool is_constant, SymbolBindingScope scope);

  const std::string& getName() const { return name; }
  std::string to_string() const;
  std::string gen_declaration(const AgentForm& agent) const;
  const std::string& gen_var_name() const { return name; }
  const VariableType& get_type() const { return type; }
  std::string gen_c_default_value() const;
  std::string gen_initial_value() const;
  SymbolBindingScope getScope() const { return scope; }
  const StateInstance& getScopeState() const;
  void set_state(const StateInstance& state) { this->state = &state; }
  std::string gen_c_type_decl() const { return type.to_c_source(); }

  static std::string scope_to_string(SymbolBindingScope scope) {
    switch(scope) {
      case SymbolBindingScope::None:
        return std::string("None");
      case SymbolBindingScope::StateLocal:
        return std::string("StateLocal");
      case SymbolBindingScope::AgentLocal:
        return std::string("AgentLocal");
      case SymbolBindingScope::Question:
        return std::string("Question");
      case SymbolBindingScope::Answer:
        return std::string("Answer");
    }
    return std::string();
  }
};

class ContextBindings {
public:
  SymbolBindingScope scope;
  std::vector<const std::vector<SymbolBinding> *> frames;

  ContextBindings(SymbolBindingScope scope) : scope(scope) {}
  SymbolBindingScope getScope() const { return scope; }
  const SymbolBinding& getBindingByName(const char * name) const;
  ContextBindings& extend(std::vector<SymbolBinding>& bindings);
};

#include "agent_form.h"

#include "source_ast.h"

class Answer;
class SourceAST_ask;
class Question {
  std::string question_name;
  const SourceAST_ask * ask_tag = nullptr;
  const AgentForm * source_agent = nullptr;
  const Answer * answer = nullptr;
  std::vector<SymbolBinding> question_scope_vars;
  std::unique_ptr<SourceAST> question_source;
  std::unique_ptr<SourceAST> return_var;
public:
  Question(ContextBindings& ctxt, xmlNodePtr node);
  Question(ContextBindings&& ctxt, xmlNodePtr node);
  Question(const Question&) = delete;
  Question(Question&&) = default;

  void set_agent(const AgentForm& agent) { source_agent = &agent; }
  void set_answer(const Answer& answer) { this->answer = &answer; }
  void set_ask_tag(const SourceAST_ask& ask) { this->ask_tag = &ask; }
  const Answer * getAnswer() const { return answer; }
  const SourceAST_ask * get_ask_tag() const { return ask_tag; }
  const std::vector<SymbolBinding> getQuestionScopeBindings() const { return question_scope_vars; }

  std::string to_string() const;

  const std::string& get_name() const { return question_name; }
  std::string gen_response_declaration() const;
  std::string gen_question_process_code() const;
  std::string gen_return_type() const;
  const SourceAST& getReturnNode() const { return *return_var; }
};

class Answer {
  const AgentForm * target_agent = nullptr;
  const Question * question = nullptr;
  std::vector<SymbolBinding> answer_scope_vars;
  std::unique_ptr<SourceAST> answer_source;
  std::unique_ptr<SourceAST> return_var;
public:
  Answer(ContextBindings& ctxt, xmlNodePtr node);
  Answer(ContextBindings&& ctxt, xmlNodePtr node) : Answer(ctxt, node) {}
  Answer(const Answer&) = delete;
  Answer(Answer&&) = default;

  std::string to_string() const;

  const SourceAST& get_source() const { return *answer_source; }
  void set_agent(const AgentForm& agent) { target_agent = &agent; }
  void set_question(const Question& question) { this->question = &question; }
  const std::string& gen_name_as_struct_member() const;
  const std::string& gen_answer_source() const;
  std::string gen_declaration() const;
  std::string gen_return_type() const;
  const SourceAST& getReturnNode() const { return *return_var; }
};

#endif
