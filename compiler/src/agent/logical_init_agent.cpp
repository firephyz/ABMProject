#include "agent/logical_init_agent.h"
#include "agent/concrete_agent.h"
#include "util.h"
#include "abmodel.h"

#include <libxml2/libxml/parser.h>
#include <string>
#include <iostream>
#include <sstream>

extern ABModel abmodel;

/******************************************************************************
 * Methods for logical initial agents                                         *
 ******************************************************************************/

LogicalInitialAgent::LogicalInitialAgent()
{}

LogicalInitialAgent::LogicalInitialAgent(const ConcreteInitialAgent& agent)
  : ConcreteInitialAgent(agent)
{
  position = agent.position.begin();
}

LogicalInitialAgent::LogicalInitialAgent(const LogicalInitialAgent& ag)
  : ConcreteInitialAgent(static_cast<ConcreteInitialAgent>(ag))
{}

// only call if you're sure the AgentPosition isn't a region
std::string
LogicalInitialAgent::gen_init_data() const
{
  std::stringstream result;
  result << this->type_unique_id << ", {";

  uint index = 0;
  for(auto& dim : position.dimensions) {
    result << dim.first_value;
    if(index != position.dimensions.size() - 1) {
      result << ", ";
    }
    index++;
  }
  result << "}, " << type_region_index;
  return result.str();
}
