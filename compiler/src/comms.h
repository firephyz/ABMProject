#ifndef COMMS_INCLUDED
#define COMMS_INCLUDED

#include <string>

// TODO can we minimize code duplication by taking advantage
// of some of the declarations in the runtime comms?
// Difficult because VHDL runtime won't have a c runtime include
// directory.

struct CommsNeighborhood {
  std::string to_c_source() const;
};

#endif
