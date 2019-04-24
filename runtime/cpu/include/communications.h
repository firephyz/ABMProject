#ifndef COMMUNICATIONS_INCLUDED
#define COMMUNICATIONS_INCLUDED

#include "spatial.h"
#include <cstdlib>

// Forward declaration
class SimSpace;
class SimCell;

/***********************************************************************
 * Models must specify these elements                                  *
 ***********************************************************************/
// Models select one of these. Each has a corresponding predicate function called by runtime
enum class NeighborhoodType {
  Square
};

// Models construct this when the runtime requests the neighborhood
// of the agent in a particular simulation cell.
typedef struct comms_neighborhood_t {
  NeighborhoodType type;
  int size;
} CommsNeighborhood;
/***********************************************************************
 * Model specifc elements done                                         *
 ***********************************************************************/

// These are the available neighborhood predicates
bool is_in_square(SimSpace& space, SimCell& receiver, SimCell& sender, int size);

bool (*getCommsPredicate(NeighborhoodType type))(SimSpace& space, SimCell& receiver, SimCell& sender, int size);

#endif
