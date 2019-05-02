#include "comms.h"
#include "abmodel.h"

#include <sstream>
#include <string>

extern ABModel abmodel;

// std::string
// CommsNeighborhood::base_c_source_wrapper(std::string&& derived_string) const
// {
//   std::stringstream result;
//   result << "CommsNeighborhood neighborhood = " << derived_string << ";";
//   return result.str();
// }

std::string
CommsNeighborhood::gen_c_declaration() const
{
  return std::string("CommsNeighborhood neighborhood");
}

std::string
Comms::NCube::gen_c_init_value() const
{
  std::stringstream result;
  result << "(CommsNeighborhood){NeighborhoodType::NCube, " << abmodel.numOfDimensions << "}";
  return result.str();
}

// std::string
// Comms::NCube::to_c_source() const
// {
//   std::stringstream result;
//   result << "{NeighborhoodType::NCube, " << size << "}";
//   return CommsNeighborhood::base_c_source_wrapper(result.str());
// }
