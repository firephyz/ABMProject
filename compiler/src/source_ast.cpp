#include "source_ast.h"

#include <sstream>
#include <iostream>

int SourceAST::print_depth = 0;
int SourceAST::start_depth = 0;

SourceAST::~SourceAST() {
  if(next.get() != nullptr) {
    delete next.get();
  }
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