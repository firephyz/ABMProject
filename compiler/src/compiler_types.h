#ifndef COMPILER_TYPES_INCLUDED
#define COMPILER_TYPES_INCLUDED

#include <string>
#include <vector>
#include <libxml2/libxml/parser.h>
//#include <libxml/parser.h>
#include <memory>
#include "source_ast.h"
#include "agent_form.h"

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
  std::string initial_value;
  bool is_constant;
public:
	SymbolBinding();
	SymbolBinding(std::string & name, VariableType type, std::string & initial_value, bool is_constant)
		: name(name)
		, type(type)
		, initial_value(initial_value)
		, is_constant(is_constant)
	{
		// allocate and copy initial value
		if (pargs.target == OutputTarget::FPGA) {
			std::cerr << "Symbol bindings for FPGA target not yet implemented\n";
			exit(-1);
		}
		else {
			if (initial_value != NULL) {
				switch (type.type) {
				case VarTypeEnum::Bool:
					this->initial_value = malloc(sizeof(bool));
					*(bool *)this->initial_value = *(bool *)initial_value;
					break;
				case VarTypeEnum::Integer:
					this->initial_value = malloc(sizeof(int));
					*(int *)this->initial_value = *(int *)initial_value;
					break;
				case VarTypeEnum::Real:
					this->initial_value = malloc(sizeof(double));
					*(double *)this->initial_value = *(double *)initial_value;
					break;
				case VarTypeEnum::String:
					this->initial_value = malloc(sizeof(char *));
					*(const char **)this->initial_value = *(const char **)initial_value;
					break;
				}
			}
		}
	}
 // SymbolBinding(std::string& name, struct VariableType type, std::string initial_value, bool is_constant);
 // ~SymbolBinding();
  std::string to_string();
  const std::string& getName() const { return name; }

  
};

class ContextBindings {
public:
	ContextBindings(int frameCount);
	ContextBindings(int frameCount, std::string conType);
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


class EnvSymbolBinding : public SymbolBinding {
	std::unique_ptr<SourceAST> EnvRule;
	public:
		EnvSymbolBinding(std::string& name, struct VariableType type, std::string initial_value, std::unique_ptr<SourceAST> rulePrt);
		EnvSymbolBinding(std::string& name, struct VariableType type, std::string initial_value);
		void updateEnvRule(std::unique_ptr<SourceAST> sast);
	
};

#endif
