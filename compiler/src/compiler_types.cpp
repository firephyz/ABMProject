#include "compiler_types.h"
#include "config.h"
#include "source_ast.h"
#include "util.h"
#include "parser.h"

#include <string>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <libxml2/libxml/parser.h>
#include <memory>

extern struct program_args_t pargs;

SymbolBinding::SymbolBinding(std::string& name, struct VariableType type, void * initial_value, bool is_constant)
  : name(name)
  , type(type)
  , initial_value(NULL)
  , is_constant(is_constant)
{
  // allocate and copy initial value
  if(pargs.target == OutputTarget::FPGA) {
    std::cerr << "Symbol bindings for FPGA target not yet implemented\n";
    exit(-1);
  }
  else {
    if(initial_value != NULL) {
      switch(type.type) {
        case VarTypeEnum::Bool:
          this->initial_value = malloc(sizeof(bool));
          *(bool *)this->initial_value = *(bool *)initial_value;
          break;
        case VarTypeEnum::Integer:
          this->initial_value = malloc(sizeof(int));
          *(int *)this->initial_value = *(int *)initial_value;
          break;
        case VarTypeEnum::Real:
          this->initial_value = malloc(sizeof(double));
          *(double *)this->initial_value = *(double *)initial_value;
          break;
        case VarTypeEnum::String:
          this->initial_value = malloc(sizeof(char *));
          *(const char **)this->initial_value = *(const char **)initial_value;
          break;
      }
    }
  }
}

VarTypeEnum strToEnum(std::string str){
  if (str == "int") 
    return VarTypeEnum::Integer; 
  if (str == "bool")
    return VarTypeEnum::Bool;
  if (str == "Real")
    return VarTypeEnum::Real;
  if (str == "String")
    return VarTypeEnum::String;
  return VarTypeEnum::Integer;
}

SymbolBinding::~SymbolBinding()
{
  free(initial_value);
}

std::string
SymbolBinding::to_string()
{
  std::stringstream result;

  result << "Symbol Binding name: \'" << name << "\' type: \'" << type.to_string() << "\'" << std::endl;

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
    if(xmlStrcmp(curNode->name, (const xmlChar *)"answer") == 0) {
      answer_source = parse_logic(ctxt.extend(question_scope_vars), xmlFirstElementChild(curNode));
    }
    else if(xmlStrcmp(curNode->name, (const xmlChar *)"questionScope") == 0) {
      parseBindings(question_scope_vars, curNode);
    }
    else if(xmlStrcmp(curNode->name, (const xmlChar *)"body") == 0) {
      question_source = parse_logic(ctxt.extend(question_scope_vars), xmlFirstElementChild(curNode));
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
  if(answer_source.get() == nullptr) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Question is missing the response logic. Needs a \'response\' tag." << std::endl;
    exit(-1);
  }
}

Question::Question(ContextBindings&& ctxt, xmlNodePtr node)
  : Question(ctxt, node)
{}

std::string
Question::to_string()
{
  std::stringstream result;

  result << "\tQuestion name: " << question_name << std::endl;
  result << "\t\t\t------ Vars ------" << std::endl;
  for(auto& binding : question_scope_vars) {
    result << "\t\t\t\t" << binding.to_string();
  }
  result << "\t\t\t------ Question Source ------" << std::endl;
  SourceAST::set_start_depth(4);
  result << question_source->print_tree();
  result << "\t\t\t------ Answer Source -------" << std::endl;
  SourceAST::set_start_depth(4);
  result << answer_source->print_tree();

  return result.str();
}