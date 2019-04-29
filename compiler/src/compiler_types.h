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
  String,
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
      case VarTypeEnum::String:
        return std::string("string");
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
      case VarTypeEnum::String:
        return std::string("const char *");
      case VarTypeEnum::State:
        return std::string("AgentState");
    }

    return std::string();
  }
};

class AgentForm;
class SymbolBinding {
  std::string name;
  struct VariableType type;
  std::string initial_value;
  bool is_constant;
public:
  SymbolBinding(std::string& name, struct VariableType type, std::string& initial_value, bool is_constant);

  const std::string& getName() const { return name; }
  std::string to_string() const;
  std::string gen_declaration(const AgentForm& agent) const;
  const std::string& gen_var_name() const { return name; }
  const VariableType& get_type() const { return type; }
  std::string gen_c_default_value() const;
};

#include "agent_form.h"

class ContextBindings {
public:
  std::vector<const std::vector<SymbolBinding> *> frames;
  const SymbolBinding& getBindingByName(const char * name) const;
  ContextBindings& extend(std::vector<SymbolBinding>& bindings);
};

#include "source_ast.h"

class Question {
  std::string question_name;
  std::vector<SymbolBinding> question_scope_vars;
  std::unique_ptr<SourceAST> question_source;
public:
  Question(ContextBindings& ctxt, xmlNodePtr node);
  Question(ContextBindings&& ctxt, xmlNodePtr node);
  Question(const Question&) = delete;
  Question(Question&&) = default;

  std::string to_string() const;

  const std::string& get_name() const { return question_name; }
};

class Answer {
  const AgentForm * target_agent = nullptr;
  const Question * question = nullptr;
  std::vector<SymbolBinding> answer_scope_vars;
  std::unique_ptr<SourceAST> answer_source;
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
};

#endif
