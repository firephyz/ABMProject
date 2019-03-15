#ifndef SOURCE_VERILOG_INCLUDED
#define SOURCE_VERILOG_INCLUDED

#include "source_ast.h"

class SourceVerilog_if : public SourceAST_if {
  std::string to_string();
};

class SourceVerilog_assignment : public SourceAST_assignment {
  std::string to_string();
};

#endif