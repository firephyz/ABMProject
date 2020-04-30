#ifndef AGENT_INTERATOR_INCLUDED
#define AGENT_INTERATOR_INCLUDED

class ConcreteInitialAgent;
class LogicalInitialAgent;

class InitialAgentIterator {
  const ConcreteInitialAgent& reference;
  LogicalInitialAgent * temp; // filled during enumeration of regions
public:
  InitialAgentIterator(const ConcreteInitialAgent& agent);
  InitialAgentIterator(const InitialAgentIterator& iter);
  ~InitialAgentIterator();

  InitialAgentIterator begin();
  InitialAgentIterator& next();
  InitialAgentIterator end();

  bool operator!=(const InitialAgentIterator& other) const;
  InitialAgentIterator& operator++();
  LogicalInitialAgent& operator*();
};

#endif
