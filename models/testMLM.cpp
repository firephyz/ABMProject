/*                           *
 *  BOIDS MLM TEST 2/16/2019 *
 *                           */

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
    integerType ID; 
    dataVector answerData; 
} Answer;

typedef struct Boid {
    integerType ID;
    AgentType type = Boid;
    std::string curState; 
    spatialVector curHeading;
    spatialVector curPosition; 
    answerBlock outputBuffer; 
} Boid;

// Prototypes
void giveAnswers();
void update(void* agent);
void* initAgent();
void giveCurHeading(Boid& agent);
void giveCurPosition(Boid& agent);

int main() {return 0;}

/*                                    *
 * Function to generate agents        *
 *                                    */
void* initAgent() {
    
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
void giveCurHeading(Boid& agent) {


}

/*                                                 *
 * Helper function to answer the current positition*
 *                                                 */
void giveCurPosition(Boid& agent) {


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

void Log(void* mlm_data) {
	
	
}
