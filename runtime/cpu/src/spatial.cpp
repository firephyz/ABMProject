#include "spatial.h"
#include "agent_model.h"

#include <vector>
#include <sstream>

extern SimSpace * space_ptr;
extern AgentModel * loaded_model;

// Required model info
//   - Explicit or implicit spatial data:
//       - Explicit: sim space data is outside of agents
//       - Implicit: sim space data is internal to agents. Moves processing to agent internals
SimSpace * SimCell::space;
SimCell::SimCell(SimSpace& space, int position_index)
  : position(new size_t[space.num_dimensions])
  , data(NULL)
{
  size_t mod_amount = 1;
  for(int dim = 0; dim < space.num_dimensions; ++dim) {
    mod_amount *= space.dimensions[dim];
  }

  for(int dim = 0; dim < space.num_dimensions; ++dim) {
    mod_amount /= space.dimensions[dim];
    position[dim] = position_index / mod_amount;
    position_index = position_index % mod_amount;
  }
}

SimCell::SimCell(SimCell&& other) noexcept
  : position(std::move(other.position))
  , data(other.data)
{}

bool SimCell::operator!=(SimCell& other)
{
  return this != &other;
}

std::string SimCell::readPosition() const {
  std::stringstream os;
	os << std::to_string(*(this->position)) << "," << std::to_string(*(this->position+1));
  return os.str();
}

size_t
SimCell::position_to_index() const
{
  return position_to_index(position);
}

size_t
SimCell::position_to_index(size_t * new_pos) const
{
  size_t result = 0;
  int scale = 1;
  for(int i = 0; i < space->num_dimensions; ++i) {
    result += new_pos[i] * scale;
    scale *= space->dimensions[i];
  }
  return result;
}

bool
SimCell::move_mlm_data(size_t * new_pos)
{
  size_t new_index = position_to_index(new_pos);
  SimCell& new_cell = space->cells[new_index];

  if(new_cell.data != nullptr) {
    return false;
  }
  else if(&new_cell == this) {
    return true;
  }
  else {
    this->data->sim_cell = &new_cell;
    new_cell.data = this->data;
    this->data = NULL;
  }

  return true;
}

SimSpace::SimSpace(AgentModel& model)
  : space_type(model.space_type)
  , num_dimensions(model.num_dims)
  , dimensions(new size_t[model.num_dims])
  , cells()
{
  int num_cells = 1;
  for(int dim = 0; dim < model.num_dims; ++dim) {
    num_cells *= model.dimensions[dim];
    dimensions[dim] = model.dimensions[dim];
  }

  // populate simulation cells and fill with init data
  cells.reserve(num_cells);
  for(int i = 0; i < num_cells; ++i) {
    cells.emplace_back(*this, i);
  }
}

SimSpace::~SimSpace() {}
