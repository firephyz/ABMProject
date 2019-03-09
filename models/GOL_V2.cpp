#include <agent_model.h>
#include <string.h>
#include<communications.h>

// stats data

// add collector stuff here

// ENV DATA - nothing really applicable to this mlm so just skeleton
	// seperate QA methods? maybe in update?
typedef struct{ // holds all basic environment related information, access tools, and function pointers
	char *id;
	char *type; //use in generator method?
	int status;
	env_buffer_out ebo;
	env_buffer_in ebi;
} env_data;

typedef struct { // holds the output end of env results, including target if any applies
	int c_xPos; // corrdinate x of center of neighborhood this data concerns
	int c_yPos; // corrdinate y of center of neighborhood this concerns

	// env parameters


} env_buffer_out;

typedef struct { // needs to hold cummulative changes to each region of environment? 
	// too env type specific to generalize easily
	
} env_buffer_in;

// AGNT Data
// need to add communication and space definitions 
// need to transpose states as helper methods (similar for their helper methods) states called in switch case of updateAgent hinged on state var in agent struct
typedef struct {
	bool status;
	char *id;
	int live_neighbors;
	int xPos;
	int yPos;
	cur_answer CA;
    CommsNeighborhood CN; // can probably fill with default
	CN.predicate = &communications.is_in_square;
	answer_package outBuffer; // need to fill in generation;

	env_data *env_data;

} mlm_data;

typedef struct {
	bool status;
	char *id;
	int xPos;
	int yPos;
} answer_package;

typedef struct { // excessivre for the single answer case, but useful structure for larger ones
	int living_neighbor_count;
} cur_answer;



mlm_data generate_gol(answer_package iap, env_data *envd, int nSize){ // initial answer package, env data, neighborghood size 
	mlm_data *gen = malloc(sizeof(mlm_data));
	(*gen).status = iap.status;
	strcpy((*gen).id, iap.id);
	(*gen).xPos = iap.xPos;
	(*gen).yPos = iap.yPos;
	(*gen).live_neighbors = 0;
	(*gen).CA.living_neighbor_count = 0;
	(*gen).CN.predicate = &is_in_square();
	(*gen).CN.size = nSize;
	(*gen).env_data = envd;
}


CommsNeighborhood giveNeighborhood((*mlm_data) ad){
		return (*ad).CN;
	
}

void * giveAnswer(mlm_data *agent_data){ // need a general way to pass useful values, write to some sort of global thing? an array of pointers?  
	(*agent_data).outBuffer.status = (*agent_data).status;
	strcpy((*agent_data).outBuffer.id , (*agent_data).id);
	(*agent_data).outBuffer.xPos = (*agent_data).xPos;
	(*agent_data).outBuffer.yPos = (*agent_data).yPos;
	return (void*)(&((*agent_data).outBuffer))
	
} 
void receiveAnswer(mlm_data *r, void *agent_ans){ // meant for handling much larger data sets with more helper functions
	 if((*(answer_package*) agent_ans).status == true){
		 (*r).CA.living_neighbor_count++;
	 }
	 
} 

void updateAgent(mlm_data *ad){
	switch((*mlm_data).status){
		case true:
			stateAlive(ad);
			break;
		case false:
			stateDead(ad);
			break;
		default:
			break;
		
	}
	
	
}

void stateAlive(mlm_data *r){ // should pull from env_data here 
	(*r).live_neighbors = (*r).CA.living_neighbor_count;
	(*r).CA.living_neighbor_count = 0;
	if(2 <= (*r).live_neighbors && (*r).live_neighbors <= 3){
		(*r).status = true;
	}
	else{
		(*r).status = false;
	}
	
}

void stateDead(mlm_data *r){ // should pull from env_data here
	(*r).live_neighbors = (*r).CA.living_neighbor_count;
	(*r).CA.living_neighbor_count = 0;
	if(2 <= (*r).live_neighbors && (*r).live_neighbors <= 3){
		(*r).status = true;
	}
	else{
		(*r).status = false;
	}
	
}

// start of the ENV 

void update_env(env_data *env) {//  need to run Q, A and update
	switch ((*env).status) {
	case 1:
		env_question(env);
		break;
	case 2:
		env_answer(env); // in agent update need to pull from known name
		break;
	default:
		env_hold(env);
		break;	
	}
}

void env_hold(env_data *env){
	// hold values
}


void env_question(env_data *env, mlm_data *dm) {
	//update buffer in
}

void env_answer(env_data *env) {
	//update buffer out
}

