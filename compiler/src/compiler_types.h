#ifndef COMPILER_TYPES_INCLUDED
#define COMPILER_TYPES_INCLUDED

#include <string>
#include <vector>

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
};

class SymbolBinding {
  std::string name;
  struct VariableType type;
  std::string initial_value;
  bool is_constant;
public:
  SymbolBinding(std::string& name, struct VariableType type, std::string initial_value, bool is_constant);
  ~SymbolBinding();

  const std::string& getName() const { return name; }
};

class ContextBindings {
public:
	ContextBindings(int frameCount);
  std::vector<std::vector<SymbolBinding> *> frames;
  const SymbolBinding& getBindingByName(const char * name) const;
};

#endif
