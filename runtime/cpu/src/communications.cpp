#include "communications.h"

#include <cmath>
#include <iostream>

bool (*getCommsPredicate(NeighborhoodType type))(SimSpace& space, SimCell& receiver, SimCell& sender, int size)
{
  switch(type) {
    case NeighborhoodType::NCube:
      return &is_in_ncube;
  }

  // Quiet the compiler
  return NULL;
}

bool is_in_ncube(SimSpace& space, SimCell& receiver, SimCell& sender, int size)
{

  for(int i = 0; i < space.num_dimensions; ++i) {
    int distance = abs(receiver.position[i] - sender.position[i]);
    if(distance > size) {
      return false;
    }
  }

  return true;
}