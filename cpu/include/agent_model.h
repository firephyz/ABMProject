#ifndef AGENT_MODEL_INCLUDED
#define AGENT_MODEL_INCLUDED

#include "communications.h"

// Verify all variables are defined when we load the model

// Agents must implement the Agent class

// #include <array>
// #include <string>
// #include <vector>

// namespace MARS {
//   enum DataTypeEnum {
//     Integer,
//     Float,
//   };

//   class DataType {
//   public:
//     static struct Integer { int32_t value };
//     static struct Float { double value };

//     DataType(std::array<uint8_t> data, DataTypeEnum type, size_t offset);
//   };
// };

// struct AgentDataField {
//   std::string name;
//   MARS::DataTypeEnum type;
//   size_t offset;
// }

// class Agent {
//   std::string agentName;
//   AgentDataField data_fields;

// }

// All ABM models must implement these types
//extern enum AgentType;

// get AgentAnswerBlock
//extern void * giveAnswer(void * mlm_data);

class AgentModel {
//  std::vector<Agent> agents;
public:
  void * giveAnswer(void * mlm_data);
  void receiveAnswer(void * mlm_data, void * answer);
  CommsNeighborhood giveNeighborhood(void * mlm_data);
};

#endif