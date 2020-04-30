#ifndef CONCRETE_AGENT
#define CONCRETE_AGENT

#include <vector>
#include <string>

#include "agent/initial_agent.h"
#include "agent/agent_iterator.h"

// overrides the initial values of the vars decared in agent_scopes.
// used during simulation initialization
struct VarValueOverride {
  std::string name; // will link and type check once we move onto code gen phase
  std::string init_value;

  VarValueOverride(xmlNodePtr node);
};

// Holds the important data required when describing an initial agent like
// the var overrides and agent type
struct ConcreteInitialAgent : public InitialAgent {
  ConcreteInitialAgent();
  ConcreteInitialAgent(xmlNodePtr node);
  ConcreteInitialAgent(const ConcreteInitialAgent& ag);

  InitialAgentIterator enumerate() const;
  void setTypeRegionIndex(size_t index) { type_region_index = index; }
  //bool operator!=(const ConcreteInitialAgent& other) const;

  // index of the concrete_agent for the init vector in which it is stored
  // under its particular type
  size_t type_region_index;
  std::vector<VarValueOverride> vars;
};

#endif
