#ifndef AGENT_FORM_INCLUDED
#define AGENT_FORM_INCLUDED

#include "compiler_types.h"
#include "c_source.h"

#include <vector>
#include <string>
#include <memory>

class StateInstance {
  const std::string state_name;
  std::vector<SymbolBinding> state_scope_vars;
  std::unique_ptr<CSource> source;
public:
  StateInstance(const std::string& name, std::unique_ptr<CSource>& source);
};

class AgentForm {
  const std::string agent_name;
  std::vector<SymbolBinding> agent_scope_vars;
  std::vector<StateInstance> states;
public:
  AgentForm(const std::string& name);

  void add_symbol_binding(const SymbolBinding& binding);
};

#endif