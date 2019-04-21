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
};

 VarTypeEnum strToEnum(std::string str);

// Additional type info required on FPGA targets
struct FPGATypeInfo {
  int bit_size;
  int dpoint_index; // fixed-point decimal index location. Indexed starting from LSB
};

struct VariableType {
  VarTypeEnum type;
  struct FPGATypeInfo fpga_info;

  VariableType()
    : type(VarTypeEnum::Bool)
    , fpga_info((struct FPGATypeInfo){-1, -1})
  {}

  std::string to_string() {
    switch(type) {
      case VarTypeEnum::Bool:
        return std::string("bool");
      case VarTypeEnum::Integer:
        return std::string("int");
      case VarTypeEnum::Real:
        return std::string("real");
      case VarTypeEnum::String:
        return std::string("string");
    }

    return std::string();
  };
};

class SymbolBinding {
  std::string name;
  struct VariableType type;
  std::string value;
  bool is_constant;
public:
  SymbolBinding(std::string& name, struct VariableType type, void * initial_value, bool is_constant);
  ~SymbolBinding();

  const std::string& getName() const { return name; }
  std::string to_string();
};

class ContextBindings {
public:
  std::vector<std::vector<SymbolBinding> *> frames;
  const SymbolBinding& getBindingByName(const char * name) const;
  ContextBindings& extend(std::vector<SymbolBinding>& bindings);
};

#include "source_ast.h"

class Question {
  std::string question_name;
  std::vector<SymbolBinding> question_scope_vars;
  std::unique_ptr<SourceAST> question_source;
  std::unique_ptr<SourceAST> answer_source;
public:
  Question(ContextBindings& ctxt, xmlNodePtr node);
  Question(ContextBindings&& ctxt, xmlNodePtr node);
  Question(const Question&) = delete;
  Question(Question&&) = default;

  std::string to_string();
};

#endif
