#ifndef CONCRETE_AGENT
#define CONCRETE_AGENT

#include <vector>
#include <string>

#include "agent/initial_agent.h"

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
  ConcreteInitialAgent(xmlNodePtr node);
  std::string gen_constructor() const {
    std::cout << "ConcreteInitialAgent\n";
    return std::string();
  }
  InitialAgentIterator enumerate() const;
  bool operator!=(const ConcreteInitialAgent& other) const;
  const std::string& getAgentName() const { return agent_type; }

  std::string agent_type;
  std::vector<VarValueOverride> vars;
  int unique_id;
};

#endif
