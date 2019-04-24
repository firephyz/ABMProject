#ifndef ABMODEL_INCLUDED
#define ABMODEL_INCLUDED

#include "agent_form.h"
#include "compiler_types.h"

#include <vector>
#include <sstream>

struct EnvironmentParameters {
	ContextBindings env;
	std::vector<std::unique_ptr<SourceAST>> LR_AST;
	std::vector<std::unique_ptr<SourceAST>> GR_AST;
public:
  ContextBindings genEnvironmentContext();
 

};

class ABModel {
  EnvironmentParameters environment;
  std::vector<AgentForm> agents;
public:
  ABModel() = default;
  ABModel(const ABModel& other) = delete;
  ABModel(ABModel&& other) = default;

  std::stringstream to_c_source();
  void add_agent(AgentForm& agent);
  void add_to_econtext(int frameSelect, char *name, char*value, char*type, bool cnst);
  ContextBindings get_env_context();
  void add_LRAST(std::unique_ptr<SourceAST> tempLR);
  void add_GRAST(std::unique_ptr<SourceAST> tempGR);
  std::vector<std::unique_ptr<SourceAST>> get_LRAST();
  std::vector<std::unique_ptr<SourceAST>> get_GRAST();

};

#endif