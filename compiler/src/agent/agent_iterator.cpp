#include "agent/logical_init_agent.h"
#include "agent/dimension.h"

/******************************************************************************
 * Iterator for initial agents. Constructs temporary agents                   *
 * for use during enumeration of regions                                      *
 ******************************************************************************/
InitialAgentIterator::InitialAgentIterator(const ConcreteInitialAgent& agent)
  : reference(agent)
  , temp(new LogicalInitialAgent(agent))
{}

InitialAgentIterator::InitialAgentIterator(const InitialAgentIterator& iter)
  : reference(iter.reference)
  , temp(new LogicalInitialAgent(*iter.temp))
{}

InitialAgentIterator::~InitialAgentIterator()
{
  free(temp);
}

InitialAgentIterator
InitialAgentIterator::begin()
{
  auto iter = InitialAgentIterator(*this);
  (*iter).position = this->reference.position.begin();
  return iter;
}

InitialAgentIterator&
InitialAgentIterator::next()
{
  for(uint dim_index = 0; dim_index < temp->position.dimensions.size(); ++dim_index) {
    const dimension& region_dim = reference.position.dimensions[dim_index];
    auto dim = temp->position.dimensions[dim_index].next(region_dim);
    // Let the last dimension roll over so we can encounter the region.end()
    if(dim == region_dim.end(dim_index) && dim_index != temp->position.dimensions.size() - 1) {
      temp->position.dimensions[dim_index] = region_dim.begin();
    }
    else {
      temp->position.dimensions[dim_index] = dim;
      break;
    }
  }

  return *this;
}

InitialAgentIterator
InitialAgentIterator::end()
{
  auto iter = InitialAgentIterator(*this);
  (*iter).position = this->reference.position.end();
  return iter;
}

bool
InitialAgentIterator::operator!=(const InitialAgentIterator& other) const
{
  return temp->position != other.temp->position;
}

InitialAgentIterator&
InitialAgentIterator::operator++()
{
  next();
  return *this;
}

LogicalInitialAgent&
InitialAgentIterator::operator*()
{
  return *temp;
}
