#include <string>
#include <regex>
#include <fstream>
#include <exception>

#include "agent_model.h"

void
AgentModel::modelLoadInitState(const char * init_filename)
{
  std::ifstream input;
  std::string line;

  // open file either from argument path or from default location
  try {
    input.exceptions(std::ios::failbit);
    if (init_filename != NULL) {
      input.open(init_filename, std::ios_base::in);
    }
    else {
      input.open(init_path, std::ifstream::in);
    }
  } catch (std::exception& e) {
    std::cerr << "Failed to open init file: " << e.what() << std::endl;
  }

  input.exceptions(std::ios::goodbit);
  std::getline(input, line);
  initial_agents_size = std::stoi(line);
  initial_agents = new InitAgent[initial_agents_size];

  size_t index = 0;
  while (!input.eof()) {
    std::getline(input, line);
    if (input.eof()) { break; }
    initial_agents[index] = InitAgent(line);
    ++index;
  }
}

InitAgent::InitAgent(std::string init)
{
  // std regex breaks, manually parse
  // <type>, {<pos>}, <init>
  size_t first_bracket = init.find('{');
  size_t second_bracket = init.find('}');
  size_t first_comma = init.find(',');
  size_t second_comma = init.find(',', second_bracket);

  std::string first_arg = init.substr(0, first_comma);
  std::string second_arg = init.substr(first_bracket+1, second_bracket - first_bracket - 1);
  std::string third_arg = init.substr(second_comma+1, init.size() - second_comma);

  agent_type = std::stoi(first_arg);
  init_data_index = std::stoi(third_arg);
}
