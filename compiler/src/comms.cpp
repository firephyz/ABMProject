#include "comms.h"
#include "abmodel.h"

#include <sstream>
#include <string>

extern ABModel abmodel;

std::string
CommsNeighborhood::base_c_source_wrapper(std::string&& derived_string) const
{
  std::stringstream result;
  auto& agent = abmodel.get_agent_by_index(agent_index);
  result << "CommsNeighborhood neighborhood_" << agent.getName() << " = " << derived_string << ";";
  return result.str();
}

std::string
Comms::NCube::to_c_source() const
{
  std::stringstream result;
  result << "{NeighborhoodType::NCube, " << size << "}";
  return CommsNeighborhood::base_c_source_wrapper(result.str());
}
