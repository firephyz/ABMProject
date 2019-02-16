#include "communications.h"

bool is_in_square(SimCell& receiver, SimCell& sender, int size) {
  int dx = abs(receiver.x - sender.x);
  int dy = abs(receiver.y - sender.y);

  if(dx <= size && dy <= size) {
    return true;
  }

  return false;
}