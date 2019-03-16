#include "agent_form.h"
#include "source_ast.h"

#include <memory>
#include <string>

StateInstance::StateInstance(const std::string& name, std::unique_ptr<SourceAST>& source)
  : state_name(name)
  , source(std::move(source))
{}

AgentForm::AgentForm(const std::string& name)
  : agent_name(name)
{}