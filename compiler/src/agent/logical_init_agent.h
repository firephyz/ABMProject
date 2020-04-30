#ifndef LOGICAL_INIT_AGENT_INCLUDED
#define LOGICAL_INIT_AGENT_INCLUDED

#include "agent/concrete_agent.h"

// Generate during enumeration of agents within a concrete_agent region
struct LogicalInitialAgent : public ConcreteInitialAgent {
  LogicalInitialAgent();
  LogicalInitialAgent(const ConcreteInitialAgent& ag);
  LogicalInitialAgent(const LogicalInitialAgent& ag);

  std::string gen_init_data() const;
};

#endif
