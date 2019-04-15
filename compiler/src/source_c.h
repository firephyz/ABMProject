#ifndef SOURCE_C_INCLUDED
#define SOURCE_C_INCLUDED

#include "source_ast.h"
#include "compiler_types.h"

#include <libxml2/libxml/parser.h>

class SourceAST_if_C : public SourceAST_if {
public:
  SourceAST_if_C(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_source();
  std::string to_string();
};

class SourceAST_assignment_C : public SourceAST_assignment {
public:
  SourceAST_assignment_C(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_source();
  std::string to_string();
};

class SourceAST_constant_C : public SourceAST_constant {
public:
  SourceAST_constant_C(xmlNodePtr node);
  std::string to_source();
  std::string to_string();
};

class SourceAST_var_C : public SourceAST_var {
public:
  SourceAST_var_C(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_source();
  std::string to_string();
};

class SourceAST_ask_C : public SourceAST_ask {
public:
  SourceAST_ask_C(xmlNodePtr node);
  std::string to_source();
  std::string to_string();
};

class SourceAST_operator_C : public SourceAST_operator {
public:
  SourceAST_operator_C(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_source();
  std::string to_string();
};

class SourceAST_return_C : public SourceAST_return {
public:
  SourceAST_return_C(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_source();
  std::string to_string();
};

class SourceAST_response_C : public SourceAST_response {
public:
  SourceAST_response_C(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_source();
  std::string to_string();
};

#endif