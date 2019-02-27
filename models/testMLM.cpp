#include <string>
#include <vector>
#include <stdint.h>
#include "agent_model.h"

using namespace std;
using integerType = uint8_t; 
using spatialVector = std::vector<integerType x, integerType y>;
using dataVector = std::vector<auto>;
using answerBlock = std::vector<Answer>; 

enum AgentType{Boid};

typedef struct Answer {
    uint8_t ID; 
    dataVector answerData; 
} Answer;

typedef struct Boid {
    integerType &ID;
    AgentType type = Boid;
    std::string curState; 
    spatialVector curHeading;
    spatialVector curPosition; 
    answerBlock outputBuffer; 
} Boid;

// Prototypes
void giveAnswers();
void update(void* agent)


/*                           *
 *  BOIDS MLM TEST 2/16/2019 *
 *                           */

int main() {return 0;}

/*                                    *
 * Function to generate agents        *
 *                                    */
std::vector<auto> generateAgents(uint32_t numOfAgents) {
    std::vector<Boid> agents;
    for (uint32_t i = 0; i < numOfAgents; i++) { 
        

    }
    
}


/*                                            *    
 * Function to place a particular agent's     *
 * answers into its ouput buffer              *
 *                                            */
void giveAnswers(void* agent) {
    answerBlock curAnswer;
    curAnswer.push_back();   
}


/*                                                 *
 * Helper function to answer the current heading   *
 *                                                 */
void giveCurHeading() {


}

/*                                                 *
 * Helper function to answer the current heading   *
 *                                                 */
void giveCurHeading() {


}



/*                                    *
 * Helper function to update an agent *
 *                                    */
void update (void* agent) {
    stateMachine();    
}

void updatePosition(void* agent) { 
    agent->    
    // Implement 

}

void stateMachine() { 
    state = getNextState(); 


}

