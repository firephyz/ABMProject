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

SimSpace& SimCell::space = *space_ptr;
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

  // create agent data if one is present. Keep NULL otherwise
  data = loaded_model->newAgent(position, this);
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
SimCell::position_to_index(const size_t * pos)
{
  size_t result = 0;
  for(int i = 0; i < space.num_dimensions; ++i) {
    result += pos[i] * space.dimensions[i];
  }
  return result;
}

const size_t *
SimCell::move_mlm_data(size_t * new_pos)
{
  size_t new_index = position_to_index(new_pos);
  SimCell& new_cell = space.cells[new_index];

  if(new_cell.data != nullptr) {
    return this->position;
  }
  else {
    new_cell.data = this->data;
    this->data = NULL;
  }

  return new_cell.position;
}

SimSpace::SimSpace(AgentModel& model) 
  : space_type(model.space_type)
  , num_dimensions(model.num_dimensions)
  , dimensions(new size_t[model.num_dimensions])
  , cells()
{
  int num_cells = 1;
  for(int dim = 0; dim < model.num_dimensions; ++dim) {
    num_cells *= model.dimensions[dim];
    dimensions[dim] = model.dimensions[dim];
  }

  cells.reserve(num_cells);
  for(int i = 0; i < num_cells; ++i) {
    cells.emplace_back(*this, i);
  }
}

SimSpace::~SimSpace() {}