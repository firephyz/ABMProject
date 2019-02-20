#include "communications.h"

#include <cmath>

// TODO finish
bool is_in_square(SimSpace& space, SimCell& receiver, SimCell& sender, int size) {
  // int mod_amount = 1;
  // for(int i = 0; i < space.model.num_dimensions; ++i) {
  //   mod_amount *= space.model.dimensions[i];
  // }

  // int receiver_pos = receiver.position_index;
  // int sender_pos = sender.position_index;

  for(int i = 0; i < space.num_dimensions; ++i) {
    // mod_amount /= space.model.dimensions[i];

    // receiver_mod_pos = receiver_pos % mod_amount;
    // sender_mod_pos = sender_pos % mod_amount;
    // dim_displacements[i] = abs(receiver_mod_pos - sender_mod_pos);
    int distance = abs(receiver.position[i] - sender.position[i]);
    if(distance > size) {
      return false;
    }
  }

  return true;
}