#include "agent_form.h"
#include "c_source.h"

#include <memory>
#include <string>

StateInstance::StateInstance(const std::string& name, std::unique_ptr<CSource>& source)
  : state_name(name)
  , source(std::move(source))
{}

AgentForm::AgentForm(const std::string& name)
  : agent_name(name)
{}