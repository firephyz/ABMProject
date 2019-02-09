#include <iostream>

// Requirements for othe team
// Question execution automatically updates corresponding variable when done
// Agents processing mulitple answers from neighboring agents:
//   - If same answer, don't run multiple times

// Processing state logic doesn't commit changes until start of next sim tick
// Extensions include variable bindings to global structures
// Spatial extension will provide simulation space and access to the agents

int main() {

    // Load model
    AgentModel model;

    SimSpace spatial(model);
    Communications::init();

    while(true) {
        // Ask every agent's question
        for(auto& agent : spatial.getAgents()) {
            // Get neighbors from comms and ask agent questions to neighbors
            Neighborhood& neighbors = Comms.getNeighborhood(agent);
            agent.askQuestions(neighbors);
        }

        for(auto& agent : spatial.getAgents()) {
            agent.processStateLogic();
        }

        for(auto& agent : spatial.getAgents()) {
            // Updates internal and local variables of the agent
            agent.update();
        }

        // Logging Extension stufffff
        // for(auto& agent : spatial.getAgents()) {
        //     agent.log();
        // }
    }

    return 0;
}