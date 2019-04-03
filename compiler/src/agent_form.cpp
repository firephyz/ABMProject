#include "agent_form.h"
#include "source_ast.h"

#include <memory>
#include <string>

StateInstance::StateInstance(const std::string& name, std::vector<SymbolBinding> scope_vars, std::unique_ptr<SourceAST>& source)
  : state_name(name)
  , state_scope_vars(std::move(scope_vars))
  , source(std::move(source))
{}

AgentForm::AgentForm(const std::string& name)
  : agent_name(name)
{}

