#include "compiler_types.h"
#include "config.h"

#include <string>
#include <iostream>

extern struct program_args_t pargs;

SymbolBinding::SymbolBinding(std::string& name, struct VariableType type, void * initial_value, bool is_constant)
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



VarTypeEnum strToEnum(std::string str){
   VarTypeEnum ret;
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
