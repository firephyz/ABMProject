#include "source_tree/source_ast.h"

#include <sstream>
#include <iostream>

// static decls inits
int SourceAST::print_depth = 0;
int SourceAST::start_depth = 0;
struct sourceast_info_t SourceAST::info;

SourceAST::~SourceAST() {
  if(next != nullptr) {
    delete next.release();
  }
}

std::string
SourceAST::to_source_start(SourceASTInfoType type, void * ptr) {
  info.type = type;
  switch(info.type) {
    case SourceASTInfoType::None:
      std::cerr << "Compiler runtime error. Cannot use SourceAST InfoType::None. What is info.data?\n";
      exit(-1);
      break;
    case SourceASTInfoType::Question:
      info.data.question = (Question *)ptr;
      break;
    case SourceASTInfoType::Answer:
      info.data.answer = (Answer *)ptr;
      break;
    case SourceASTInfoType::StateInstance:
      info.data.state = (StateInstance *)ptr;
      break;
  }
  info.is_setup = true;
  std::string result = this->to_source();
  info.is_setup = false;
  return result;
}

std::string
SourceAST::to_string_prefix() {
  std::stringstream result;
  for(int i = 0; i < start_depth; ++i) {
    result << "\t";
  }
  for(int i = 0; i < print_depth; ++i) {
    result << "|\t";
  }
  return result.str();
}

std::string
SourceAST::print_tree() {
  std::stringstream result;

  result << SourceAST::to_string_prefix();
  SourceAST::to_string_fall();
  result << this->to_string();
  SourceAST::to_string_rise();

  if(next != nullptr) {
    result << next->print_tree();
  }

  return result.str();
}

std::string
SourceAST_var::gen_type() const
{
  return binding->get_type().to_c_source();
}

std::string
SourceAST_var::gen_declaration() const
{
  std::stringstream result;
  result << gen_type() << " " << binding->getName() << ";";
  return result.str();
}

std::string
SourceAST_var::gen_c_default_value() const
{
  return binding->gen_c_default_value();
}

std::string
SourceAST_constant::gen_declaration() const
{
  std::stringstream result;
  result << gen_type() << " " << "TODO;";
  return result.str();
}

std::string
SourceAST_constant::gen_c_default_value() const
{
  switch(type) {
    case ConstantType::NoInit:
      return "0xA5A5";
    case ConstantType::Integer:
      return "0";
    case ConstantType::Real:
      return "0.0";
    case ConstantType::Bool:
      return "false";
    case ConstantType::State:
      return "AgentState::STATE_NONE";
  }
  return std::string();
}
