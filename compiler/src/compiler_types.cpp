#include "compiler_types.h"
#include "config.h"
#include "source_ast.h"
#include "util.h"
#include "parser.h"
#include "agent_form.h"

#include <string>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <libxml2/libxml/parser.h>
#include <memory>
#include <sstream>

extern struct program_args_t pargs;

SymbolBinding::SymbolBinding(std::string& name, struct VariableType type, std::string& initial_value, bool is_constant)
  : name(name)
  , type(type)
  , initial_value(initial_value)
  , is_constant(is_constant)
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
  if (str == "String")
    return VarTypeEnum::String;
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
      parseBindings(question_scope_vars, curNode);
    }
    else if(xmlStrcmp(curNode->name, (const xmlChar *)"body") == 0) {
      // change parser state so we limit the allowed AST
      parser.set_state(ParserState::Questions);
      question_source = parser.parse_logic(ctxt.extend(question_scope_vars), xmlFirstElementChild(curNode));
    }
    else {
      std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Unrecognized tag in question \'" << question_name << "\'." << std::endl;
      exit(-1);
    }

    curNode = xmlNextElementSibling(curNode);
  }

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
  result << "\t\t\t------ Vars ------" << std::endl;
  for(auto& binding : question_scope_vars) {
    result << "\t\t\t\t" << binding.to_string();
  }
  result << "\t\t\t------ Question Source ------" << std::endl;
  SourceAST::set_start_depth(4);
  result << question_source->print_tree();

  return result.str();
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
      parseBindings(answer_scope_vars, curNode);
    }
    else if(xmlStrcmp(curNode->name, (const xmlChar *)"body") == 0) {
      // change parser state so we limit the allowed AST
      parser.set_state(ParserState::Questions);
      answer_source = parser.parse_logic(ctxt.extend(answer_scope_vars), xmlFirstElementChild(curNode));
    }
    else {
      std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Unrecognized tag in answer: \'" << xml_attr->children->content << "\'" << std::endl;
      exit(-1);
    }

    curNode = xmlNextElementSibling(curNode);
  }

  if(answer_source.get() == nullptr) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Answer is missing the answer body logic. Needs a \'body\' tag." << std::endl;
    exit(-1);
  }
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
  SourceAST::set_start_depth(4);
  result << answer_source->print_tree();

  return result.str();
}