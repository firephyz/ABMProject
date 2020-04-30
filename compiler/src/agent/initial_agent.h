#ifndef INITIAL_AGENT_INCLUDED
#define INITIAL_AGENT_INCLUDED

#include <libxml2/libxml/parser.h>
#include <vector>
#include <string>
#include <iostream>

#include "agent/position.h"
#include "compiler_types.h"

// stores location information
struct InitialAgent {
  InitialAgent() = default;
  InitialAgent(const InitialAgent& ag);

  bool operator<(const InitialAgent& other) const;
  bool operator!=(const InitialAgent& other) const;
  const std::string& getName() const { return agent_type; }
  const size_t getUniqueId() const { return type_unique_id; }
  const std::vector<SymbolBinding>& getAgentScopeBindings() const;

  std::string agent_type;
  size_t type_unique_id;
  AgentPosition position;
  const AgentForm * agent;
};

#endif
