#include "agent_model.h"
#include "spatial.h"

#include <iostream>

// Requirements for othe team
// Question execution automatically updates corresponding variable when done
// Agents processing mulitple answers from neighboring agents:
//   - If same answer, don't run multiple times

// Processing state logic doesn't commit changes until start of next sim tick
// Extensions include variable bindings to global structures
// Spatial extension will provide simulation space and access to the agents

// Q & A Dichotomy: Do agents ask questions or give answers? no.

extern AgentModel model;

int main() {

  // Load model dynamic library
  // giveAnswer
  // giveNeighborhood
  // recieveAnswer
  // num_dimensions and dimensions

  SimSpace space(model);
  //Communications::init();

  while(true) {
    // Ask every agent's question
    for(auto& sender : space.cells) {
      void * agent_answer = model.giveAnswer(sender.mlm_data);
      for(auto& receiver : space.cells) {
        if(sender != receiver) {
          CommsNeighborhood n = model.giveNeighborhood(receiver.mlm_data);
          if(n.predicate(space, receiver, sender, n.size)) {
            model.receiveAnswer(receiver.mlm_data, agent_answer);
          }
        }
      }
    }

    for(auto& cell : space.cells) {
      updateAgent(cell.mlm_data);
    }

    // Logging Extension stufffff
    // for(auto& agent : space.getAgents()) {
    //     agent.log();
    // }
  }

  return 0;
}