#ifndef SOURCE_C_INCLUDED
#define SOURCE_C_INCLUDED

#include "source_ast.h"
#include "compiler_types.h"

#include <libxml2/libxml/parser.h>

class SourceAST_if_C : public SourceAST_if {
protected:
  std::string to_source() const;
public:
  SourceAST_if_C(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_string();
};

class SourceAST_assignment_C : public SourceAST_assignment {
protected:
  std::string to_source() const;
public:
  SourceAST_assignment_C(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_string();
};

class SourceAST_constant_C : public SourceAST_constant {
  const AgentForm * states_agent = nullptr;
protected:
  std::string to_source() const;
public:
  SourceAST_constant_C(xmlNodePtr node);
  std::string to_string();
};

class SourceAST_var_C : public SourceAST_var {
protected:
  std::string to_source() const;
public:
  SourceAST_var_C(const ContextBindings& ctxt, xmlNodePtr node);
  SourceAST_var_C(const SymbolBinding * binding) : SourceAST_var(binding) {}
  std::string to_string();
  std::string get_var_c_name() const;
};

class SourceAST_ask_C : public SourceAST_ask {
protected:
  std::string to_source() const;
public:
  SourceAST_ask_C(xmlNodePtr node, const SourceAST_var * binding);
  std::string to_string();
  std::string get_target_var_c_name() const;
};

class SourceAST_operator_C : public SourceAST_operator {
protected:
  std::string to_source() const;
public:
  SourceAST_operator_C(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_string();
};

class SourceAST_return_C : public SourceAST_return {
protected:
  std::string to_source() const;
public:
  SourceAST_return_C(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_string();
};

class SourceAST_response_C : public SourceAST_response {
protected:
  std::string to_source() const;
public:
  SourceAST_response_C(const ContextBindings& ctxt, xmlNodePtr node);
  std::string to_string();
};

#endif