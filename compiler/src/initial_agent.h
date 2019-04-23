#ifndef INITIAL_AGENT_INCLUDED
#define INITIAL_AGENT_INCLDUED

#include <libxml2/libxml/parser.h>
#include <vector>
#include <string>

// overrides the initial values of the vars decared in agent_scopes.
// used during simulation initialization
struct VarValueOverride {
  std::string var_id; // will link and type check once we move onto code gen phase
  std::string value;

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
    bool is_binary() const {
      switch(relation) {
        case relation_t::LT:
        case relation_t::GT:
          return false;
        default:
          return true;
      }
    }

    // 'or equal to' is captured by incrementing or decrementing the value given in the XML model.
    // i.e. we are using exclusive relations. >=5,<10 stores 5 and 9
    int first_value = -1;
    int second_value = -1; // only present if relation is a binary one
    relation_t relation {relation_t::None};
    type_t position_type {dimension::type_t::None};
  };

  AgentPosition() = default;
  AgentPosition(xmlNodePtr node, const std::string& str);
  bool overlaps(const AgentPosition& other) const;

  std::vector<dimension> position;
};

// stores location information and variable overrides for the starting start of an agent
struct InitialAgent {
  InitialAgent(xmlNodePtr node);
  bool operator<(const InitialAgent& other) const {
    auto other_iter = other.position.position.begin();
    for(auto& dim : position.position) {
      if(dim > *other_iter) return false;
    }
    return true;
  }

  AgentPosition position;
  std::string agent_type;
  std::vector<VarValueOverride> vars;
};

#endif
