#include "agent_form.h"
#include "source_ast.h"
#include "comms.h"
#include "abmodel.h"

#include <memory>
#include <string>
#include <sstream>

extern ABModel abmodel;

StateInstance::StateInstance(const std::string& name)
  : state_name(name)
{}

const std::vector<SymbolBinding>&
StateInstance::getStateScopeBindings() const
{
  return state_scope_vars;
}

void
StateInstance::add_logic(std::unique_ptr<SourceAST>&& source)
{
  this->source = std::move(source);
}

std::string
StateInstance::to_string()
{
  std::stringstream result;

  result << "\tState name: " << state_name << std::endl;
  result << "\t\t\t------ Vars ------" << std::endl;
  for(auto& binding : state_scope_vars) {
    result << "\t\t\t\t" << binding.to_string();
  }
  result << "\t\t\t------ Source -------" << std::endl;
  SourceAST::set_start_depth(4);
  result << source->print_tree();

  return result.str();
}

std::string
StateInstance::gen_state_enum_name(const std::string& agent_name) const
{
  std::stringstream result;
  result << "STATE_" << agent_name << "_" << state_name;
  return result.str();
}

AgentForm::AgentForm(const std::string& name)
  : agent_name(name)
  , neighborhood(nullptr)
  , log_en(false)
{}

void
AgentForm::set_neighborhood(std::unique_ptr<CommsNeighborhood>&& n)
{
  neighborhood = std::move(n);
}

const std::vector<SymbolBinding>&
AgentForm::getAgentScopeBindings() const
{
  return agent_scope_vars;
}

StateInstance&
AgentForm::add_state(StateInstance&& state)
{
  states.push_back(std::move(state));
  return states[states.size() - 1];
}

void
AgentForm::resolve_ask_links()
{
  for(SourceAST_ask * node : parser.nodes) {
    for(auto& q : questions) {
      if(q->get_name() == node->getQuestionName()) {
        // give ask node the shared_ptr to Question
        node->setQuestion(q);
        q->set_ask_tag(*node);
        break;
      }
    }

    if(node->getQuestion() == nullptr) {
      std::cerr << "Could not link ask node with question name \'" << node->getQuestionName();
      std::cerr << "\' with a question in agent \'" << agent_name << "\'." << std::endl;
      exit(-1);
    }
  }
  // empty nodes for next agent to be parsed
  parser.nodes.clear();
}

ContextBindings
AgentForm::genContextBindings(StateInstance& state)
{
  ContextBindings result(SymbolBindingScope::StateLocal);
  result.frames.push_back(&state.getStateScopeBindings());
  result.frames.push_back(&agent_scope_vars);
  return result;
}

ContextBindings
AgentForm::genContextBindings()
{
  ContextBindings result(SymbolBindingScope::AgentLocal);
  result.frames.push_back(&agent_scope_vars);
  return result;
}

std::string
AgentForm::to_string()
{
  std::stringstream result;

  result << "Name: " << agent_name << std::endl;
  result << "\t------ Vars ------" << std::endl;
  for(auto& binding : agent_scope_vars) {
    result << "\t\t" << binding.to_string();
  }
  result << "\t------ States ------" << std::endl;
  for(auto& state : states) {
    result << "\t" << state.to_string() << std::endl;
  }
  result << "\t------ Questions -------" << std::endl;
  for(auto& question : questions) {
    result << "\t" << question->to_string() << std::endl;
  }
  result << "\t------ Answers -------" << std::endl;
  for(auto& answer : answers) {
    result << "\t" << answer->to_string() << std::endl;
  }

  // reset SourceAST printer
  SourceAST::set_start_depth(0);

  return result.str();
}

std::string
AgentForm::gen_enum_type_name() const
{
  return std::string("AGENT_") + agent_name;
}

std::string
AgentForm::gen_mlm_data_struct() const
{
  std::stringstream result;

  // Declare struct for holding answers
  result << gen_answer_struct();

  // Declare struct for holding question reponses
  result << gen_responses_struct();

  // Generate data struct for questions locals
  result << "struct mlm_data_" << agent_name << "_questions {\n";
  for(auto& question : questions) {
    result << "\tstruct " << question->get_name() << "_t {\n";
    for(auto& local : question->getQuestionScopeBindings()) {
      result << "\t\t" << local.gen_c_type_decl() << " " << local.gen_var_name() << ";\n";
    }
    result << "\t} " << question->get_name() << ";\n";
  }
  result << "};\n\n";

  // generate data structs for each state (for local variables)
  for(auto& state : states) {
    result << "\
struct mlm_data_" << agent_name << "_" << state.getName() << " {\n";
    for(auto& variable : state.getStateScopeBindings()) {
      result << "\t" << variable.gen_declaration(*this) << "\n";
    }
    result << "};\n";
    result << "\n";
  }

  // generate mlm_data for specific agent
  result << "\
struct " << gen_mlm_data_string() << " : public mlm_data {\n";
  result << "\tstatic struct answer_" << agent_name << " answers;\n";
  result << "\tstatic struct responses_" << agent_name << " responses;\n";
  result << "\tstruct mlm_data_" << agent_name << "_questions q_locals;\n";
  for(auto& variable : agent_scope_vars) {
    result << "\t" << variable.gen_declaration(*this) << "\n";
  }
  for(auto& state : states) {
    // place struct type declaration
    result << "\tunion {\n";
    result << "\t\tstruct mlm_data_" << agent_name << "_" << state.getName();
    result << " locals_" << state.getName() << ";\n";
    result << "\t};\n";
    // place struct name
  }
  result << "\n";
  
  // constructor
  //result << "\t AgentType * typePtr = (AgentType*)malloc(sizeof(AgentType));\n";
	//result << "\t *typePtr = AgentType::" << gen_enum_type_name() << ";\n";
  result << "\t" << gen_mlm_data_string() << "(SimCell * sim_cell)\n";
  result << "\t\t: mlm_data(sim_cell, AgentType::" << gen_enum_type_name()  << ", (CommsNeighborhood)" << neighborhood->gen_c_init_value() << ")\n";
  result << "\t{\n";
  result << "\t\tanswers = answer_" << agent_name << "(AgentType::AGENT_" << agent_name << ");\n";
  result << "\t}\n";
  result << "\
  void record_answers() {\n";
  for(auto& answer : answers) {
    result << "\t\tanswers." << answer->gen_name_as_struct_member() << " = ";
    result << answer->gen_answer_source() << ";\n";
  }
  result << "\t}\n";
  result << "\
  answer_block * give_answers() const { return (answer_block *)&answers; }\n\
  void receive_answers(answer_block * answer);\n\
  void process_questions();\n";
  for(auto& question : questions) {
    result << "\t" << question->gen_return_type() << " process_question_" << question->get_name();
    result << "(" << "mlm_data_" << this->getName() << "_questions::" << question->get_name() << "_t * locals)\n;";
  }
  result << "};\n";
  result << "struct answer_" << agent_name << " " << gen_mlm_data_string() << "::answers(AgentType::AGENT_" << agent_name << ");" << std::endl;
  result << "struct responses_" << agent_name << " " << gen_mlm_data_string() << "::responses;" << std::endl;
  return result.str();
}

std::string
AgentForm::gen_responses_struct() const
{
  std::stringstream result;
  result << "struct responses_" << agent_name << " {\n";
  for(auto& question : questions) {
    result << "\t" << question->gen_response_declaration() << ";\n";
  }
  result << "};\n";
  result << "\n";
  return result.str();
}

std::string
AgentForm::gen_receive_answer_code() const
{
  std::stringstream result;

  // Code to get answer from answer_block and place it into the agent resonses
  result << "void\n";
  result << gen_mlm_data_string() << "::receive_answers(answer_block * answers)\n{\n";
  result << "\tswitch(answers->type_tag) {\n";
  for(auto& agent : abmodel.agents) {
    result << "\t\t" << "case AgentType::AGENT_" << agent.getName() << ": {\n";
    result << "\t\t\tanswer_" << agent.getName() << " * agent_answers = static_cast<answer_" << agent.getName() << " *>(answers);\n";
    for(auto& question : questions) {
      result << "\t\t\t" << "responses." << question->get_name() << " = ";
      result << "agent_answers->" << agent.getName() << "_" << question->get_name() << ";\n";
    }
    result << "\t\t\tbreak;\n\t\t}\n";
  }
  result << "\t}\n";
  result << "}\n\n";

  // Code to do question post-processing on the responses
  result << "void\n";
  result << gen_mlm_data_string() << "::process_questions()\n{\n";
  for(auto& question : questions) {
    result << "\tprocess_question_" << question->get_name() << "(&q_locals." << question->get_name() << ");\n";
  }
  result << "}\n\n";

  // Generate functions to process each question; each needs its on scope for variables
  for(auto& question : questions) {
    result << question->gen_question_process_code();
  }

  return result.str();
}

std::string 
AgentForm::gen_log_code() const 
{
	std::stringstream result;  
  result <<"\
	if (*(data->type) == AgentType::" << gen_enum_type_name() << ") {\n\
		logStr << \":\" << \"\\\\\" << data->sim_cell->readPosition();\n\
	}";
  return result.str();
}

std::string 
AgentForm::gen_mlm_data_string() const
{
	return std::string("mlm_data_") + agent_name;
}

std::string
AgentForm::gen_answer_struct() const
{
  std::stringstream result;
  std::string struct_name = "answer_" + agent_name;
  result << "struct " << struct_name << " : public answer_block {\n";
  for(auto& answer : answers) {
    result << "\t" << answer->gen_declaration() << ";\n";
  }
  result << "\n";
  result << "\t" << struct_name << "(AgentType type) : answer_block(type) {}\n";
  result << "};\n";
  result << "\n";
  return result.str();
}

std::string
AgentForm::gen_reset_question_locals_code() const
{
  std::stringstream result;
  for(auto& question : questions) {
    for(auto& binding : question->getQuestionScopeBindings()) {
      result << "\t" << "mlm_data_" << agent_name << "_questions." << question->get_name() << "." << binding.getName() << " = " << binding.gen_c_default_value() << ";\n";
    }
  }
  return result.str();
}
