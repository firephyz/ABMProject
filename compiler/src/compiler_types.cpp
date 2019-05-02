#include "compiler_types.h"
#include "config.h"
#include "source_ast.h"
#include "util.h"
#include "parser.h"
#include "agent_form.h"
#include "abmodel.h"

#include <string>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <libxml2/libxml/parser.h>
#include <memory>
#include <sstream>

extern ABModel abmodel;
extern struct program_args_t pargs;

SymbolBinding::SymbolBinding(std::string& name, struct VariableType type, std::string& initial_value, bool is_constant, SymbolBindingScope scope)
  : name(name)
  , agent_index(0xA5A5A5A5)
  , type(type)
  , initial_value(initial_value)
  , is_constant(is_constant)
  , scope(scope)
{
  // allocate and copy initial value
  if(pargs.target == OutputTarget::FPGA) {
    std::cerr << "Symbol bindings for FPGA target not yet implemented\n";
    exit(-1);
  }
}

VarTypeEnum strToEnum(std::string& str){
  if (str == "int")
    return VarTypeEnum::Integer;
  if (str == "bool")
    return VarTypeEnum::Bool;
  if (str == "real")
    return VarTypeEnum::Real;
  if (str == "state")
    return VarTypeEnum::State;
  
  std::cerr << "Unknown type string \'" << str <<"\'." << std::endl;
  exit(-1);
}

std::string
SymbolBinding::to_string() const
{
  std::stringstream result;

  result << "Symbol Binding name: \'" << name << "\' type: \'" << type.to_string() << "\'" << std::endl;

  return result.str();
}

std::string
SymbolBinding::gen_c_default_value() const
{
  if(initial_value.empty()) {
    switch(type.type) {
      case VarTypeEnum::Integer:
        return "0";
      case VarTypeEnum::Real:
        return "0.0";
      case VarTypeEnum::Bool:
        return "false";
      case VarTypeEnum::State:
        return "FIXME_INIT_STATE";
    }
  }
  else {
    if(type.type == VarTypeEnum::State) {
      std::string agent_name = getAgent().getName();
      std::cout << agent_name << std::endl;
      return std::string("AgentState::STATE_") + agent_name + "_" + initial_value;
    }
    else {
      return initial_value;
    }
  }
  return std::string();
}

const AgentForm&
SymbolBinding::getAgent() const
{
  if(agent_index == 0xA5A5A5A5) {
    std::cerr << "Compiler runtime error: Attempting to get an agent from a SymbolBinding which doesn't have an associated agent.\n";
    exit(-1);
  }
  return abmodel.agents[agent_index];
}

std::string
SymbolBinding::gen_declaration(const AgentForm& agent) const
{
  std::string type_part = type.to_c_source();
  std::stringstream result;
  result << type_part << " " << name;
  if(!initial_value.empty()) {
    std::string init_str;
    if(type.type == VarTypeEnum::State) {
      init_str = std::string("AgentState::STATE_") + agent.getName() + "_" + initial_value;
    }
    else {
      init_str = initial_value;
    }
    result << " = " << init_str << ";";
  }
  else {
    result << ";";
  }
  return result.str();
}

const StateInstance&
SymbolBinding::getScopeState() const
{
  if(state_index == 0xA5A5A5A5) {
    if(scope != SymbolBindingScope::StateLocal) {
      std::cerr << "Error: Attempting to get the state of variable \'" << name << "\'" << " which was not declared in state scope or is was not handled properly in code.\n";
      exit(-1);
    }
    else {
      std::cerr << "Compiler runtime error: Variable binding \'" << name << " declared in a state scope doesn't have a corresponding state link.\n";
      exit(-1);
    }
  }
  return abmodel.agents[agent_index].getStates()[state_index];
}

const SymbolBinding&
ContextBindings::getBindingByName(const char * name) const
{
  for(auto& frame : frames) {
    auto result = std::find_if(frame->begin(),
      frame->end(),
      [&] (const SymbolBinding& b) {
        return strcmp(name, b.getName().c_str()) == 0;
      }
    );

    if(result != frame->end()) {
      return *result;
    }
  }

  std::cerr << "Binding \'" << name << "\' not found in the given context." << std::endl;
  exit(-1);
}

ContextBindings&
ContextBindings::extend(std::vector<SymbolBinding>& bindings)
{
  frames.push_back(&bindings);
  return *this;
}

Question::Question(ContextBindings& ctxt, xmlNodePtr node)
{
  // Parse question name
  auto xml_attr = xmlGetAttribute(node, "name");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Question needs a \'name\' attribute." << std::endl;
    exit(-1);
  }
  question_name = std::string((const char *)xml_attr->children->content);

  xmlNodePtr curNode = xmlFirstElementChild(node);
  while(curNode != NULL) {
    if(xmlStrcmp(curNode->name, (const xmlChar *)"scope") == 0) {
      parseBindings(question_scope_vars, curNode, SymbolBindingScope::Question);
    }
    else if(xmlStrcmp(curNode->name, (const xmlChar *)"body") == 0) {
      // change parser state so we limit the allowed AST
      parser.set_state(ParserState::Questions);
      question_source = parser.parse_logic(ctxt.extend(question_scope_vars), xmlFirstElementChild(curNode));
    }
    else if(xmlStrcmp(curNode->name, (const xmlChar *)"return") == 0) {
      parser.set_state(ParserState::Questions);
      return_var = dispatch_on_logic_tag(ctxt.extend(question_scope_vars), xmlFirstElementChild(curNode));
      if(return_var->get_type() != ASTNodeType::Node_var) {
        util::error(curNode) << "Question return must be a var tag type.\n";
        exit(-1);
      }
    }
    else {
      std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Unrecognized tag in question \'" << question_name << "\'." << std::endl;
      exit(-1);
    }

    curNode = xmlNextElementSibling(curNode);
  }

  // Questions need bodies to return responses
  if(question_source.get() == nullptr) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Question is missing the question body logic. Needs a \'body\' tag." << std::endl;
    exit(-1);
  }
}

Question::Question(ContextBindings&& ctxt, xmlNodePtr node)
  : Question(ctxt, node)
{}

std::string
Question::to_string() const
{
  std::stringstream result;

  result << "\tQuestion name: " << question_name << std::endl;
  result << "\t\t         addr: " << this << std::endl;
  result << "\t\tA addr: " << answer << std::endl;
  result << "\t\t\t------ Vars ------" << std::endl;
  for(auto& binding : question_scope_vars) {
    result << "\t\t\t\t" << binding.to_string();
  }
  result << "\t\t\t------ Question Source ------" << std::endl;
  SourceAST::set_start_depth(4);
  result << question_source->print_tree();
  result << "\t\t\t------ Question Return ------" << std::endl;
  SourceAST::set_start_depth(4);
  result << return_var->print_tree();

  return result.str();
}

std::string
Question::gen_response_declaration() const
{
  std::string decl = answer->gen_return_type() + " " + question_name;
  return std::move(decl);
}

std::string
Question::gen_question_process_code() const
{
  std::stringstream result;

  result << this->gen_return_type() << "\n";
  result << source_agent->gen_mlm_data_string() << "::process_question_" << this->get_name();
  result << "(" << "mlm_data_" << source_agent->getName() << "_questions::" << this->get_name() << "_t * locals)\n";
  result << "{\n";

  // Gen source
  result << util::indent(question_source->to_source_start(SourceASTInfoType::Question, (void *)this));
  result << "\n}\n\n";

  return result.str();
}

std::string
Question::gen_return_type() const
{
  return return_var->gen_type();
}

Answer::Answer(ContextBindings& ctxt, xmlNodePtr node)
{
  // Agent and question will remain null until parsing is done.
  // Only then can we link them together
  auto xml_attr = xmlGetAttribute(node, "agent");
  if(xml_attr == NULL) {
    util::error(node) << "Answer needs a \'agent\' attribute." << std::endl;
    exit(-1);
  }
  std::string agent_name((const char *)xml_attr->children->content);

  xml_attr = xmlGetAttribute(node, "question");
  if(xml_attr == NULL) {
    util::error(node) << "Answer needs a corresponding\'question\' attribute." << std::endl;
    exit(-1);
  }
  std::string question_name((const char *)xml_attr->children->content);

  // store some data so we can link answer to question and agent later after parsing is done
  parser.answers_to_be_linked.emplace_back(this, question_name, agent_name);

  xmlNodePtr curNode = xmlFirstElementChild(node);
  while(curNode != NULL) {
    if(xmlStrcmp(curNode->name, (const xmlChar *)"scope") == 0) {
      parseBindings(answer_scope_vars, curNode, SymbolBindingScope::Answer);
    }
    else if(xmlStrcmp(curNode->name, (const xmlChar *)"body") == 0) {
      // change parser state so we limit the allowed AST
      parser.set_state(ParserState::Questions);
      answer_source = parser.parse_logic(ctxt.extend(answer_scope_vars), xmlFirstElementChild(curNode));
    }
    else if(xmlStrcmp(curNode->name, (const xmlChar *)"return") == 0) {
      parser.set_state(ParserState::Questions);
      return_var = dispatch_on_logic_tag(ctxt.extend(answer_scope_vars), xmlFirstElementChild(curNode));
      if(return_var->get_type() != ASTNodeType::Node_var) {
        util::error(curNode) << "Question return must be a var tag type.\n";
        exit(-1);
      }
    }
    else {
      std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Unrecognized tag in answer: \'" << xml_attr->children->content << "\'" << std::endl;
      exit(-1);
    }

    curNode = xmlNextElementSibling(curNode);
  }

  // Answers don't need bodies. They can just return stuff
  // if(answer_source.get() == nullptr) {
  //   std::cerr << "<" << xmlGetLineNo(node) << "> " << "Answer is missing the answer body logic. Needs a \'body\' tag." << std::endl;
  //   exit(-1);
  // }

  // // Make sure answer source has a correct form. Can only return a constant or variable
  // if(answer_source->get_type() != ASTNodeType::Node_return) {
  //   util::error(node) << "Answer body must immediately return its result.\n";
  //   exit(-1);
  // }

  ASTNodeType type = return_var->get_type();
  if((type != ASTNodeType::Node_var) &&
     (type != ASTNodeType::Node_constant)) {
    util::error(node) << "Answer body can only return a constant or variable.\n";
    exit(-1);
  }
}

const std::string&
Answer::gen_answer_source() const
{
  if(return_var->get_type() == ASTNodeType::Node_constant) {
    SourceAST_constant& constant_node = *(SourceAST_constant *)return_var.get();
    return constant_node.getValue();
  }
  else if(return_var->get_type() == ASTNodeType::Node_var) {
    SourceAST_var& var_node = *(SourceAST_var *)return_var.get();
    return var_node.getBinding().gen_var_name();
  }
  else {
    std::cerr << "Compiler error in \'Answer::gen_answer_source()\'. Answer result returns something other than a constant or variable." << std::endl;
    exit(-1);
  }
}

std::string
Answer::gen_declaration() const
{
  std::string type_str = gen_return_type();
  std::string default_value = return_var->gen_c_default_value();
  return type_str + " " + gen_name_as_struct_member() + " = " + default_value;
}

std::string
Answer::gen_return_type() const
{
  return return_var->gen_type();
}

const std::string&
Answer::gen_name_as_struct_member() const
{
  static std::string name = target_agent->getName() + "_" + question->get_name();
  return name;
}

std::string
Answer::to_string() const
{
  std::stringstream result;

  result << "\tAnswer: to " << target_agent->getName() << ", for question " << question->get_name() << std::endl;
  result << "\t\t  addr: " << this << std::endl;
  result << "\t\tQ addr: " << question << std::endl;
  result << "\t\t\t------ Vars ------" << std::endl;
  for(auto& binding : answer_scope_vars) {
    result << "\t\t\t\t" << binding.to_string();
  }
  result << "\t\t\t------ Answer Source -------" << std::endl;
  if(answer_source != nullptr) {
    SourceAST::set_start_depth(4);
    result << answer_source->print_tree();
  }

  result << "\t\t\t------ Answer Return ------" << std::endl;
  SourceAST::set_start_depth(4);
  result << return_var->print_tree();

  return result.str();
}