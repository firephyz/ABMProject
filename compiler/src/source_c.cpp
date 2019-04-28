#include "source_c.h"
#include "parser.h"
#include "util.h"
#include "debug.h"

#include <libxml2/libxml/parser.h>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cstring>

#ifndef VERBOSE_AST_GEN
#error "Code which has the potential to utilize VERBOSE_AST_GEN debug\
 option does not have it defined."
#endif

SourceAST_if_C::SourceAST_if_C(const ContextBindings& ctxt, xmlNodePtr node)
{
  xmlNodePtr child = xmlFirstElementChild(node);   

  while (child != NULL) { 
    if (xmlStrcmp(child->name, (const xmlChar*)"predicate") == 0) { 
      predicate = parser.parse_logic(ctxt, child);
    }
    else if(xmlStrcmp(child->name, (const xmlChar*)"then") == 0) {
      then_clause = parser.parse_logic(ctxt, child);
    }
    else if(xmlStrcmp(child->name, (const xmlChar*)"else") == 0) {
      else_clause = parser.parse_logic(ctxt, child);
    }
    else {
      std::cerr << "Invalid xml tag \'" << child->name << "\' under \'<if>\' tag.\n";
      exit(-1);
    }

    child = xmlNextElementSibling(child);
  }

  #if VERBOSE_AST_GEN
    std::cout << "If" << std::endl;
  #endif
}

std::string
SourceAST_if_C::to_source()
{
  std::stringstream result;
  result << "if (" << predicate->to_source() << ") { " << then_clause->to_source() << " }";
  if(else_clause != nullptr) {
    result << " else { " << else_clause->to_source() << " }";
  }
  return result.str();
}

std::string
SourceAST_if_C::to_string()
{
  std::stringstream result;

  result << "IF" << std::endl;
  result << predicate->print_tree();
  result << then_clause->print_tree();
  if(else_clause != nullptr) {
    result << else_clause->print_tree();
  }

  return result.str();
}

SourceAST_assignment_C::SourceAST_assignment_C(const ContextBindings& ctxt, xmlNodePtr node)
{
  xmlNodePtr curNode = xmlFirstElementChild(node);
  if(xmlStrcmp(curNode->name, (const xmlChar *)"var") != 0) {
    std::cerr << "Assignment tag is missing the left-hand variable." << std::endl;
    exit(-1);
  }

  xmlAttrPtr xml_attr = xmlGetAttribute(curNode, "id");
  if(xml_attr == NULL) {
    std::cerr << "Var tag in assigment is missing its id attribute." << std::endl;
    exit(-1);
  }
  const char * symbol_name = (const char *)xml_attr->children->content;
  binding = &ctxt.getBindingByName(symbol_name);

  curNode = xmlNextElementSibling(curNode);
  if(xmlStrcmp(curNode->name, (const xmlChar *)"ask") == 0) {
    type = AssignmentValueType::CommsAnswer;
    value_answer = std::make_unique<SourceAST_ask_C>(curNode);
  }
  else if(xmlStrcmp(curNode->name, (const xmlChar *)"constant") == 0) {
    type = AssignmentValueType::Expression;
    value_expr = std::make_unique<SourceAST_constant_C>(curNode);
  }
  else if(xmlStrcmp(curNode->name, (const xmlChar *)"operator") == 0) {
    type = AssignmentValueType::Expression;
    value_expr = std::move(dispatch_on_logic_tag(ctxt, curNode));
  }
  else {
    std::cerr << "Unrecognized assignment value type for variable \'" << symbol_name << "\'." << std::endl;
    exit(-1);
  }

  #if VERBOSE_AST_GEN
    std::cout << "Assignment: " << this->to_source() << std::endl;
  #endif
}

std::string
SourceAST_assignment_C::to_source()
{
  std::stringstream result;
  std::string value_string = type == AssignmentValueType::Expression ? 
    value_expr->to_source() :
    (type == AssignmentValueType::CommsAnswer ?
      (value_answer.get() == nullptr ?
        "ANSWER" :
        value_answer->to_source()) :
      "NoInit");
  result << binding->getName() << " = " << value_string << ";";
  return result.str();
}

std::string
SourceAST_assignment_C::to_string()
{
  std::stringstream result;

  result << "ASSIGN var=\'" << binding->getName() << "\'" << std::endl;
  switch(type) {
    case AssignmentValueType::NoInit:
      break;
    case AssignmentValueType::Expression:
      result << value_expr->print_tree();
      break;
    case AssignmentValueType::CommsAnswer:
      result << value_answer->print_tree();
      break;
  }

  return result.str();
}

SourceAST_constant_C::SourceAST_constant_C(xmlNodePtr node)
{
  xmlAttrPtr xml_attr = xmlGetAttribute(node, "type");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Constant doesn't have a type attribute." << std::endl;
    exit(-1);
  }

  const char * type_string = (const char *)xml_attr->children->content;
  if(strcmp(type_string, "int") == 0) {
    type = ConstantType::Integer;
  }
  else if(strcmp(type_string, "real") == 0) {
    type = ConstantType::Real;
  }
  else if(strcmp(type_string, "bool") == 0) {
    type = ConstantType::Bool;
  }
  else {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Constant is of unknown type \'" << type_string << "\'" << std::endl;
    exit(-1);
  }

  xml_attr = xmlGetAttribute(node, "value");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Constant doesn't have a value attribute." << std::endl;
    exit(-1);
  }

  // // TODO add type checking for the value attr content strings for the integers and reals
  // switch(type) {
  //   case ConstantType::NoInit:
  //     break; // won't get here
  //   case ConstantType::Integer:
  //     content = malloc(sizeof(int));
  //     *(int *)content = atoi((const char *)xml_attr->children->content);
  //     break;
  //   case ConstantType::Real:
  //     content = malloc(sizeof(double));
  //     *(double *)content = atof((const char *)xml_attr->children->content);
  //     break;
  //   case ConstantType::Bool:
  //     if(xmlStrcmp(xml_attr->children->content, (const xmlChar *)"false") == 0) {
  //       *(bool *)content = false;
  //     }
  //     else if(xmlStrcmp(xml_attr->children->content, (const xmlChar *)"true") == 0) {
  //       *(bool *)content = true;
  //     }
  //     else {
  //       std::cerr << "<" << xmlGetLineNo(node) << "> " << "Bool constant has invalid value." << std::endl;
  //       exit(-1);
  //     }
  //     break;
  value = std::string((const char *)xml_attr->children->content);

  #if VERBOSE_AST_GEN
    std::cout << "Constant: " << type_string << ", " << value << std::endl;
  #endif
}

std::string
SourceAST_constant_C::to_source()
{
  return value;
}

std::string
SourceAST_constant_C::to_string()
{
  std::stringstream result;

  result << "CONSTANT type=\'";
  switch(type) {
    case ConstantType::NoInit:
      break;
    case ConstantType::Integer:
      result << "int";
      break;
    case ConstantType::Real:
      result << "real";
      break;
    case ConstantType::Bool:
      result << "bool";
      break;
  }
  result << "\' value=\'" << value << "\'" << std::endl;

  return result.str();
}

SourceAST_var_C::SourceAST_var_C(const ContextBindings& ctxt, xmlNodePtr node)
{
  xmlAttrPtr xml_attr = xmlGetAttribute(node, "id");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Var doesn't have an \'id\' attribute." << std::endl;
    exit(-1);
  }

  binding = &ctxt.getBindingByName((const char *)xml_attr->children->content);

  #if VERBOSE_AST_GEN
    std::cout << "Var: " << binding->getName() << std::endl;
  #endif
}

std::string
SourceAST_var_C::to_source()
{
  return binding->getName();
}

std::string
SourceAST_var_C::to_string()
{
  std::stringstream result;

  result << "VAR name=\'" << binding->getName() << "\'" << std::endl;

  return result.str();
}

SourceAST_ask_C::SourceAST_ask_C(xmlNodePtr node)
{
  auto xml_attr = xmlGetAttribute(node, "name");
  if(xml_attr == NULL) {
    std::cerr << "Ask tag in assigment is missing the question name." << std::endl;
    exit(-1);
  }
  const char * qname = (const char *)xml_attr->children->content;
  this->question_name = std::string(qname);

  // TODO get matching question from abmodel
  parser.nodes.push_back(this);

  #if VERBOSE_AST_GEN
    std::cout << "Ask: " << question_name << std::endl;
  #endif
}

std::string
SourceAST_ask_C::to_source()
{
  // TODO finish
  return std::string();
}

std::string
SourceAST_ask_C::to_string()
{
  std::stringstream result;

  result << "ASK: \'" << question_name << "\' " << question.get() << std::endl;

  return result.str();
}

SourceAST_operator_C::SourceAST_operator_C(const ContextBindings& ctxt, xmlNodePtr node)
{
  auto xml_attr = xmlGetAttribute(node, "type");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Operator is missing type attribute." << std::endl;
    exit(-1);
  }

  OperatorTypeEnum type_enum = OperatorTypeEnum::NoInit;
  int num_args = -1;
  if(xmlStrcmp(xml_attr->children->content, (const xmlChar *)"add") == 0) {
    type_enum = OperatorTypeEnum::Add;
    num_args = 2;
  }
  else if(xmlStrcmp(xml_attr->children->content, (const xmlChar *)"equal") == 0) {
    type_enum = OperatorTypeEnum::Equal;
    num_args = 2;
  }
  else if(xmlStrcmp(xml_attr->children->content, (const xmlChar *)"not") == 0) {
    type_enum = OperatorTypeEnum::Not;
    num_args = 1;
  }
  else if(xmlStrcmp(xml_attr->children->content, (const xmlChar *)"or") == 0) {
    type_enum = OperatorTypeEnum::Or;
    num_args = 2;
  }
  else {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Unknown operator type attribute \'" << xml_attr->children->content << "\'" << std::endl;
    exit(-1);
  }

  type = OperatorType(type_enum, num_args);
  xmlNodePtr child = xmlFirstElementChild(node);
  int num_args_processed = 0;

  while(child != NULL) {
    args.push_back(dispatch_on_logic_tag(ctxt, child));
    child = xmlNextElementSibling(child);
    ++num_args_processed;
  }

  if(num_args_processed != num_args) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Operator requires " << num_args << " many arguments." << std::endl;
    exit(-1);
  }

  #if VERBOSE_AST_GEN
    std::cout << "Operator: " << type.to_string() << std::endl;
  #endif
}

std::string
SourceAST_operator_C::to_source()
{
  return std::string();
}

std::string
SourceAST_operator_C::to_string()
{
  std::stringstream result;

  result << "OPERATOR type=\'" << type.to_string() << "\'" << std::endl;
  for(auto& arg : args) {
    result << arg->print_tree();
  }

  return result.str();
}

SourceAST_return_C::SourceAST_return_C(const ContextBindings& ctxt, xmlNodePtr node)
{
  xmlNodePtr curNode = xmlFirstElementChild(node);
  if(curNode == NULL) {
    std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Return requires a value to return." << std::endl;
    exit(-1);
  }

  value = dispatch_on_logic_tag(ctxt, curNode);

  if(xmlNextElementSibling(curNode) != NULL) {
    std::cerr << "<" << xmlGetLineNo(curNode) << "> " << "Return requires only a single value element to return." << std::endl;
    exit(-1);
  }

  #if VERBOSE_AST_GEN
    std::cout << "Return" << std::endl;
  #endif
}

std::string
SourceAST_return_C::to_source()
{
  return std::string();
}

std::string
SourceAST_return_C::to_string()
{
  std::stringstream result;

  result << "RETURN" << std::endl;
  result << value->print_tree();

  return result.str();
}

SourceAST_response_C::SourceAST_response_C(const ContextBindings& ctxt, xmlNodePtr node)
{
  #if VERBOSE_AST_GEN
    std::cout << "Response" << std::endl;
  #endif
}


std::string
SourceAST_response_C::to_source()
{
  return std::string();
}

std::string
SourceAST_response_C::to_string()
{
  std::stringstream result;

  result << "RESPONSE" << std::endl;

  return result.str();
}