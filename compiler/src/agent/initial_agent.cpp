#include "agent/concrete_agent.h"
#include "agent/initial_agent.h"
#include "compiler_types.h"

#include <vector>

InitialAgent::InitialAgent(const InitialAgent& ag)
  : agent_type(ag.agent_type)
  , type_unique_id(ag.type_unique_id)
  , position(ag.position)
  , agent(ag.agent)
{}

const std::vector<SymbolBinding>&
InitialAgent::getAgentScopeBindings() const
{
  return agent->getAgentScopeBindings();
}
