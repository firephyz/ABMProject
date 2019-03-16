#ifndef COMPILER_TYPES_INCLUDED
#define COMPILER_TYPES_INCLUDED

#include <string>

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
};

class SymbolBinding {
  std::string name;
  struct VariableType type;
  void * initial_value;
  bool is_constant;
public:
  SymbolBinding(std::string& name, struct VariableType type, void * initial_value, bool is_constant);
  ~SymbolBinding();
};

#endif
