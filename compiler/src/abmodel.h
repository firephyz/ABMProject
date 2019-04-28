#ifndef ABMODEL_INCLUDED
#define ABMODEL_INCLUDED

#include "agent_form.h"
#include "compiler_types.h"
#include "initial_agent.h"

#include <vector>
#include <string>

struct EnvironmentParameters {
	ContextBindings env;
	bool wrap;
	int dims;
	//std::vector<std::unique_ptr<SourceAST>> LR_AST;
	//std::vector<std::unique_ptr<SourceAST>> GR_AST;
public:
  ContextBindings genEnvironmentContext();
  

 

};
struct InitialState {
	std::vector<int> dimension_sizes;
	std::vector<ConcreteInitialAgent> agents;
};
class ABModel {
  EnvironmentParameters environment;
  
  std::vector<AgentForm> agents;
	public:
		ABModel() = default;
		ABModel(const ABModel& other) = delete;
		ABModel(ABModel&& other) = default;
		InitialState init;
		// for code gen
		std::string to_c_source();
		std::vector<std::string> gen_initial_agent_defs();
		std::string gen_spatial_enum();
		std::string gen_space_dims();
		std::string gen_space_size();
		std::string gen_mlm_data_struct();

		void add_agent(AgentForm& agent);
		void add_to_econtext(int frameSelect, char *name, char*value, char*type, bool cnst);
		ContextBindings get_env_context();
		bool wrapState();
		int dimCount();
		void setDim(int D);
		void setWrap(bool W);
		//void add_LRAST(std::unique_ptr<SourceAST> tempLR);
		//void add_GRAST(std::unique_ptr<SourceAST> tempGR);
		//std::vector<std::unique_ptr<SourceAST>> get_LRAST();
		//std::vector<std::unique_ptr<SourceAST>> get_GRAST();


  std::string to_string();
};

#endif
