#ifndef COMMS_INCLUDED
#define COMMS_INCLUDED

#include <string>

// TODO can we minimize code duplication by taking advantage
// of some of the declarations in the runtime comms?
// Difficult because VHDL runtime won't have a c runtime include
// directory.

struct CommsNeighborhood {
  const size_t agent_index;

  CommsNeighborhood(const size_t agent_index)
    : agent_index(agent_index)
  {}

  virtual std::string to_c_source() const = 0;
  std::string base_c_source_wrapper(std::string&& derived_string) const;
};

namespace Comms {
  struct NCube : public CommsNeighborhood {
    const int size; // radius of cube from center

    NCube(const size_t agent_index, int size)
      : CommsNeighborhood(agent_index)
      , size(size)
    {}
    std::string to_c_source() const;
  };
}

#endif
