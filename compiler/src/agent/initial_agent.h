#ifndef INITIAL_AGENT_INCLUDED
#define INITIAL_AGENT_INCLUDED

#include <libxml2/libxml/parser.h>
#include <vector>
#include <string>
#include <iostream>

#include "agent/position.h"

// stores location information and variable overrides for the starting start of an agent
struct InitialAgent {
  virtual std::string gen_constructor() const {
    std::cerr << "Compiler runtime error: Failed to call derived class InitialAgent::gen_constructor.\n";
    exit(-1);
  }
  bool operator<(const InitialAgent& other) const;
  static bool base_neq(const InitialAgent& a, const InitialAgent& other);
  virtual const std::string& getAgentName() const = 0;

  AgentPosition position;
};

// Holds only an absolute position and a reference to an actual concrete initial agent
// whose position region contains this agent
struct ConcreteInitialAgent;
struct LogicalInitialAgent : public InitialAgent {
  LogicalInitialAgent(const ConcreteInitialAgent& agent);
  std::string gen_constructor() const;
  void next(); // increments position to next in the concrete agent region
  bool operator!=(const LogicalInitialAgent& other) const;
  const std::string& getAgentName() const;

  const ConcreteInitialAgent& actual;
};

class ConcreteInitialAgent;

class InitialAgentIterator {
  LogicalInitialAgent agent; // filled during enumeration of regions
public:
  InitialAgentIterator(const ConcreteInitialAgent& agent) : agent(agent) {}
  InitialAgentIterator& begin();
  InitialAgentIterator& end();

  bool operator!=(const InitialAgentIterator& other) const;
  InitialAgentIterator& operator++();
  LogicalInitialAgent& operator*();
};

#endif
