#ifndef INITIAL_AGENT_INCLUDED
#define INITIAL_AGENT_INCLUDED

#include <libxml2/libxml/parser.h>
#include <vector>
#include <string>
#include <iostream>

// overrides the initial values of the vars decared in agent_scopes.
// used during simulation initialization
struct VarValueOverride {
  std::string name; // will link and type check once we move onto code gen phase
  std::string init_value;

  VarValueOverride(xmlNodePtr node);
};

// Used to specify the extend to with the initial agent declaration covers
// the simulation space. Can be used to declared large regions as a certain
// kind of agent with certain values or can specify single positions.
struct AgentPosition {
  enum class relation_t {
    None,
    // Unary relation operators
    LT,
    GT,
    // Binary relational operators
    LTLT,
    LTGT,
    GTLT,
    GTGT,
  };
  struct dimension {
    enum class type_t {
      None,
      Absolute_Position,
      Relational_Position,
      All,
    };

    dimension(const std::string& str);
    bool overlaps(const struct dimension& other) const;
    bool operator<(const struct dimension& other) const;\
    bool operator>(const struct dimension& other) const {
      return !((*this < other) || (*this == other));
    }
    bool operator==(const struct dimension& other) const {
      return (first_value == other.first_value) &&
        (second_value == other.second_value) &&
        (relation == other.relation) &&
        (position_type == other.position_type);
    }
    bool operator!=(const struct dimension& other) const { return !(*this == other); }
    bool is_binary() const {
      switch(relation) {
        case relation_t::LT:
        case relation_t::GT:
          return false;
        default:
          return true;
      }
    }
    dimension& operator=(const dimension& other) {
      first_value = other.first_value;
      second_value = other.second_value;
      relation = other.relation;
      position_type = other.position_type;
      return *this;
    }
    // only defined on Absolute_Position types
    dimension& operator-(int a);
    dimension& operator+(int a);
    dimension begin() const;
    dimension end(int dim_index) const;
    dimension next(const dimension& region_dim) const;

    // 'or equal to' is captured by incrementing or decrementing the value given in the XML model.
    // i.e. we are using exclusive relations. >=5,<10 stores 5 and 9
    int first_value = -1;
    int second_value = -1; // only present if relation is a binary one
    relation_t relation {relation_t::None};
    type_t position_type {dimension::type_t::None};
  };

  AgentPosition() = default;
  AgentPosition(std::vector<dimension> pos)
    : dimensions(pos)
    , is_region_value(false)
  {}
  AgentPosition(xmlNodePtr node, const std::string& str);
  AgentPosition& operator=(const AgentPosition& other) {
    dimensions = other.dimensions;
    is_region_value = other.is_region_value;
    return *this;
  }
  bool overlaps(const AgentPosition& other) const;
  bool is_region() const {
    return is_region_value; // pre-computed at construction time
  }
  AgentPosition begin() const;
  AgentPosition end() const;

  std::vector<dimension> dimensions;
  bool is_region_value = false;
};

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

struct ConcreteInitialAgent;

// Holds only an absolute position and a reference to an actual concrete initial agent
// whose position region contains this agent
struct LogicalInitialAgent : public InitialAgent {
  LogicalInitialAgent(const ConcreteInitialAgent& agent);
  std::string gen_constructor() const;
  void next(); // increments position to next in the concrete agent region
  bool operator!=(const LogicalInitialAgent& other) const;
  const std::string& getAgentName() const;

  const ConcreteInitialAgent& actual;
};

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
