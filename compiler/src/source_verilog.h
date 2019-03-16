#ifndef SOURCE_VERILOG_INCLUDED
#define SOURCE_VERILOG_INCLUDED

#include "source_ast.h"

#include <libxml/parser.h>

class SourceAST_if_Verilog : public SourceAST_if {
public:
  SourceAST_if_Verilog(xmlNodePtr node);
  std::string to_string();
};

class SourceAST_assignment_Verilog : public SourceAST_assignment {
public:
  SourceAST_assignment_Verilog(xmlNodePtr node);
  std::string to_string();
};

#endif