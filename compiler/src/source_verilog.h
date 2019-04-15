#ifndef SOURCE_VERILOG_INCLUDED
#define SOURCE_VERILOG_INCLUDED

#include "source_ast.h"

#include <libxml/parser.h>

class SourceAST_if_Verilog : public SourceAST_if {
public:
  SourceAST_if_Verilog(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_string();
};

class SourceAST_assignment_Verilog : public SourceAST_assignment {
public:
  SourceAST_assignment_Verilog(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_string();
};

class SourceAST_constant_Verilog : public SourceAST_assignment {
public:
  SourceAST_constant_Verilog(xmlNodePtr node);
  std::string to_string();
};

class SourceAST_var_Verilog : public SourceAST_assignment {
public:
  SourceAST_var_Verilog(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_string();
};

class SourceAST_operator_Verilog : public SourceAST_operator {
public:
  SourceAST_operator_Verilog(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_string();
};

class SourceAST_return_Verilog : public SourceAST_return {
public:
  SourceAST_return_Verilog(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_string();
};

class SourceAST_response_Verilog : public SourceAST_response {
public:
  SourceAST_response_Verilog(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_string();
};

#endif