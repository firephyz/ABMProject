#include "agent_model.h"
#include "spatial.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <string>
#include <dlfcn.h>
#include <cstdlib>

#define DLSYM_ERROR_CHECK(varName, symbol)\
if(varName == NULL) {\
  std::cerr << "Error: Could not locate symbol \'" << symbol << "\'\n";\
  char * error = dlerror();\
  if(error != NULL) {\
    std::cerr << error << std::endl;\
  }\
  exit(-1);\
}

//int SimAgent::num_dimensions = -1; // placeholder for actual number

// TODO Allow setting this with command line arguments
//const char * model_lib_path = "../../../models/test/test_model.so";

// Globals loaded with libdl with symbols provided by given model.so file
AgentModel * loaded_model                                                                                 = NULL;
void *              (*modelNewAgentPtr)(AgentModel * this_class, void *, const SimCell& sim_cell)                                  = NULL;
void *              (*modelGiveAnswerPtr)(AgentModel * this_class, void * mlm_data)                       = NULL;
void                (*modelReceiveAnswerPtr)(AgentModel * this_class, void * mlm_data, void * answer)     = NULL;
CommsNeighborhood&  (*modelGiveNeighborhoodPtr)(AgentModel * this_class, void * mlm_data)                 = NULL;
void                (*modelUpdateAgentPtr)(AgentModel * this_class, void * mlm_data)                      = NULL;
std::string         (*modelLogPtr)(AgentModel * this_class, void * mlm_data);
void                (*modelTickPtr)(AgentModel * this_class);


// Output mangled symbols to temporary file to read in later
// TODO Make this better so we for sure only grab the symbols we need.
// Then nm grep won't need occassional adjusting (as mentioned below
// in the if statement before the return)
std::vector<std::string>
extractMangledSymbols(const char * model_path) {

  // Construct bash command to read shared library
  std::stringstream command;
  const char *symbols[] =\
  {
    "modelGiveAnswer",
    "modelNewAgent",
    "modelGiveNeighborhood",
    "modelUpdateAgent",
    "modelReceiveAnswer",
    "modelLog",
    "modelTick"
  };
  const char * sed_part = "sed \'s/.*T //g\'";
  const char * grep_part = "grep -E \"_ZN10AgentModel.*(";
  const char * grep_end = ")\"";
  const char * output_part = " > _phobos_symbols.txt";
  command << "nm " << model_path << " | " << grep_part;
  for(uint i = 0; i < (sizeof(symbols) / sizeof(char *)); ++i) {
    command << symbols[i];
    if(i < (sizeof(symbols) / sizeof(char *)) - 1) {
      command << "|";
    }
  }
  command << grep_end << " | " << sed_part << output_part;
  // This command parses the model library to find the mangled symbol names
  // of the symbols we need to load in
  std::system(command.str().c_str());

  // Read in mangled symbols
  std::vector<std::string> result;
  std::ifstream symbols_in("_phobos_symbols.txt");
  std::string temp_symbol;
  char c = symbols_in.get();
  while(!symbols_in.eof()) {
    if(c == '\n') {
      result.push_back(std::move(temp_symbol));
      temp_symbol = std::string("");
    }
    else {
      temp_symbol.push_back(c);
    }
    c = symbols_in.get();
  }
  std::system("rm _phobos_symbols.txt");

  // Without this, the program will load symbols it doesn't actually need
  if(result.size() != (sizeof(symbols) / sizeof(const char *))) {
    std::cerr << "Error: One or more symbols are missing in model file!" << std::endl;
    exit(-1);
  }

  return result;
}

void loadModelSymbol(void * model_handle, std::string& symbol) {
  if(symbol.find("loaded_model") != std::string::npos) {
    loaded_model = (AgentModel *)dlsym(model_handle, symbol.c_str());
    DLSYM_ERROR_CHECK(loaded_model, symbol);
  }
  else if(symbol.find("modelGiveAnswer") != std::string::npos) {
    modelGiveAnswerPtr = (void * (*)(AgentModel *, void *))dlsym(model_handle, symbol.c_str());
    DLSYM_ERROR_CHECK(modelGiveAnswerPtr, symbol);
  }
  else if(symbol.find("modelNewAgent") != std::string::npos) {
    modelNewAgentPtr = (void * (*)(AgentModel *, void *, const SimCell& sim_cell))dlsym(model_handle, symbol.c_str());
    DLSYM_ERROR_CHECK(modelNewAgentPtr, symbol);
  }
  else if(symbol.find("modelGiveNeighborhood") != std::string::npos) {
    modelGiveNeighborhoodPtr = (CommsNeighborhood& (*)(AgentModel *, void *))dlsym(model_handle, symbol.c_str());
    DLSYM_ERROR_CHECK(modelGiveNeighborhoodPtr, symbol);
  }
  else if(symbol.find("modelReceiveAnswer") != std::string::npos) {
    modelReceiveAnswerPtr = (void (*)(AgentModel *, void *, void *))dlsym(model_handle, symbol.c_str());
    DLSYM_ERROR_CHECK(modelReceiveAnswerPtr, symbol);
  }
  else if(symbol.find("modelUpdateAgent") != std::string::npos) {
    modelUpdateAgentPtr = (void (*)(AgentModel *, void *))dlsym(model_handle, symbol.c_str());
    DLSYM_ERROR_CHECK(modelUpdateAgentPtr, symbol);
  }
  else if(symbol.find("modelLog") != std::string::npos) {
    modelLogPtr = (std::string (*)(AgentModel *, void *))dlsym(model_handle, symbol.c_str());
  } 
  else if(symbol.find("modelTick") != std::string::npos) {
  	modelTickPtr = (void (*)(AgentModel *))dlsym(model_handle, symbol.c_str());
  }
  else {
    std::cerr << "Could not match symbol \'" << symbol << "\' in the mangled symbols list." << std::endl;
    exit(-1);
  }
}

void * loadModel(const char * model_path) {
  std::vector<std::string> symbols = extractMangledSymbols(model_path);
  symbols.emplace_back("loaded_model");

  // Load model dynamic library
  void * model_handle = dlopen(model_path, RTLD_NOW);
  if(model_handle == NULL) {
    std::cerr << "Error: Could not open model file \'" << model_path << "\'" << std::endl;
    char * error = dlerror();
    if(error != NULL) {
      std::cerr << error << std::endl;
    }
    exit(-1);
  }

  // Load all of the symbols
  for(auto& symbol : symbols) {
    loadModelSymbol(model_handle, symbol);
  }

  return model_handle;
}

int main(int argc, char** argv) {
  
  const char* model_lib_path;
  model_lib_path = *(argv + 1);
  void * model_handle = loadModel(model_lib_path);

  // set dimensions used by SimAgents
  //SimAgent::num_dimensions = loaded_model->num_dimensions;

  SimSpace space(*loaded_model);
  
  // Print out Model name
  std::cout << loaded_model->model_name << std::endl;

  int limit = 3;
  int it = 0;
  while(it < limit) {
    // Ask every agent's question
    for(auto& sender : space.cells) {
      if(sender.is_empty()) continue; // skip if no agent is present
      void * agent_answer = loaded_model->giveAnswer(sender.mlm_data);
      for(auto& receiver : space.cells) {
        if(sender != receiver) {
          if(receiver.is_empty()) continue; // skip if no agent is present
          CommsNeighborhood& n = loaded_model->giveNeighborhood(receiver.mlm_data);
          auto commsPredicate = getCommsPredicate(n.type);
          if(commsPredicate(space, receiver, sender, n.size)) {
            loaded_model->receiveAnswer(receiver.mlm_data, agent_answer);
          }
        }
      }
    }

    for(auto& cell : space.cells) {
      if(cell.is_empty()) continue; // skip empty cells
      loaded_model->updateAgent(cell.mlm_data);
    }

    // Logging Extension stufffff
    for(auto& cell : space.cells) {
      if(cell.is_empty()) continue; // skip empty cells
      std::cout << "" <<loaded_model->Log(cell.mlm_data);
    }

    // Call the model tick function
  	loaded_model->Tick();
    it++;
  }

  // Close the model library
  if(dlclose(model_handle)) {
    std::cerr << "Error: Failed to close model handle" << std::endl;
    exit(-1);
  }

  return 0;
}
