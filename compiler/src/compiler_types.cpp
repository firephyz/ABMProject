#include "compiler_types.h"
#include "config.h"

#include <string>
#include <cstring>
#include <algorithm>
#include <iostream>

extern struct program_args_t pargs;

SymbolBinding::SymbolBinding(std::string& name, struct VariableType type, std::string initial_value, bool is_constant)
  : name(name)
  , type(type)
  , initial_value(NULL)
  , is_constant(is_constant)
{
  // allocate and copy initial value
  if(pargs.target == OutputTarget::FPGA) {
    std::cerr << "Symbol bindings for FPGA target not yet implemented\n";
    exit(-1);
  }
  else {
    if(initial_value != NULL) {
      switch(type.type) {
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

VarTypeEnum strToEnum(std::string str){
  if (str == "int") 
    return VarTypeEnum::Integer; 
  if (str == "bool")
    return VarTypeEnum::Bool;
  if (str == "Real")
    return VarTypeEnum::Real;
  if (str == "String")
    return VarTypeEnum::String;
  return VarTypeEnum::Integer;
}

SymbolBinding::~SymbolBinding()
{
  free(initial_value);
}

const SymbolBinding&
ContextBindings::getBindingByName(const char * name) const
{
  for(auto& frame : frames) {
    auto result = std::find_if(frame->begin(),
      frame->end(),
      [&] (const SymbolBinding& b) {
        return strcmp(name, b.getName().c_str()) == 0;
      }
    );

    if(result != frame->end()) {
      return *result;
    }
  }

  std::cerr << "Binding \'" << name << "\' not found in the given context." << std::endl;
  exit(-1);
}


ContextBindings::ContextBindings(int frameCount) {
	for (int i = 0; i < frameCount; i++) {
		ContextBindings::frames.push_back(std::vector<SymbolBinding> SB);
	}

}