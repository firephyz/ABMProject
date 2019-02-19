#ifndef COMMUNICATIONS_INCLUDED
#define COMMUNICATIONS_INCLUDED

#include "spatial.h"

// Models construct this when the runtime requests the neighborhood
// of the agent in a particular simulation cell.
typedef struct {
  bool (*predicate)(SimSpace& space, SimCell& receiver, SimCell& sender, int size);
  int size;
} CommsNeighborhood;

bool is_in_square(SimSpace& space, SimCell& receiver, SimCell& sender, int size);

#endif