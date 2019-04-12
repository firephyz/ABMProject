#include "source_c.h"
#include "parser.h"

#include <libxml2/libxml/parser.h>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>

SourceAST_if_C::SourceAST_if_C(const ContextBindings& ctxt, xmlNodePtr node)
{
  xmlNodePtr child = xmlFirstElementChild(node);   

  while (child != NULL) { 
    if (xmlStrcmp(child->name, (const xmlChar*)"predicate") == 0) { 
      predicate = parse_logic(ctxt, child);
    }
    else if(xmlStrcmp(child->name, (const xmlChar*)"then") == 0) {
      then_clause = parse_logic(ctxt, child);
    }
    else if(xmlStrcmp(child->name, (const xmlChar*)"else") == 0) {
      else_clause = parse_logic(ctxt, child);
    }
    else {
      std::cerr << "Invalid xml tag \'" << child->name << "\' under \'<if>\' tag.\n";
      exit(-1);
    }

    child = xmlNextElementSibling(child);
  }

  #ifdef VERBOSE_AST_GEN
    std::cout << "If" << std::endl;
  #endif
}

std::string
SourceAST_if_C::to_string()
{
  std::stringstream result;
  result << "if (" << predicate->to_string() << ") { " << then_clause->to_string() << " }";
  if(else_clause != nullptr) {
    result << " else { " << else_clause->to_string() << " }";
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
    type = AssignmentValueType::Constant;
    value_constant = std::make_unique<SourceAST_constant_C>(curNode);
  }
  else {
    std::cerr << "Unrecognized assignment value type for variable \'" << symbol_name << "\'." << std::endl;
    exit(-1);
  }

  #ifdef VERBOSE_AST_GEN
    std::cout << "Assignment: " << this->to_string() << std::endl;
  #endif
}

std::string
SourceAST_assignment_C::to_string()
{
  std::stringstream result;
  std::string value_string = type == AssignmentValueType::Constant ? 
    value_constant->to_string() :
    (type == AssignmentValueType::CommsAnswer ?
      (value_answer.get() == nullptr ?
        "ANSWER" :
        value_answer->to_string()) :
      "NoInit");
  result << binding->getName() << " = " << value_string << ";";
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
  if(xmlStrcmp(xml_attr->children->content, (const xmlChar *)"int") == 0) {
    type = ConstantType::Integer;
  }
  else if(xmlStrcmp(xml_attr->children->content, (const xmlChar *)"real") == 0) {
    type = ConstantType::Real;
  }
  else if(xmlStrcmp(xml_attr->children->content, (const xmlChar *)"bool") == 0) {
    type = ConstantType::Bool;
  }
  else {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Constant is of unknown type \'" << xml_attr->children->content << "\'" << std::endl;
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

  #ifdef VERBOSE_AST_GEN
    std::cout << "Constant: " << type_string << ", " << value << std::endl;
  #endif
}

std::string
SourceAST_constant_C::to_string()
{
  return value;
}

SourceAST_var_C::SourceAST_var_C(const ContextBindings& ctxt, xmlNodePtr node)
{
  xmlAttrPtr xml_attr = xmlGetAttribute(node, "id");
  if(xml_attr == NULL) {
    std::cerr << "<" << xmlGetLineNo(node) << "> " << "Var doesn't have an \'id\' attribute." << std::endl;
    exit(-1);
  }

  binding = &ctxt.getBindingByName((const char *)xml_attr->children->content);

  #ifdef VERBOSE_AST_GEN
    std::cout << "Var: " << binding->getName() << std::endl;
  #endif
}

std::string
SourceAST_var_C::to_string()
{
  return binding->getName();
}

SourceAST_ask_C::SourceAST_ask_C(xmlNodePtr node)
{
  auto xml_attr = xmlGetAttribute(node, "name");
  if(xml_attr == NULL) {
    std::cerr << "Ask tag in assigment is missing the question name." << std::endl;
    exit(-1);
  }
  const char * question_name = (const char *)xml_attr->children->content;

  // TODO get matching question from abmodel

  #ifdef VERBOSE_AST_GEN
    std::cout << "Ask: " << question_name << std::endl;
  #endif
}

std::string
SourceAST_ask_C::to_string()
{
  // TODO finish
  return std::string();
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

  #ifdef VERBOSE_AST_GEN
    std::cout << "Operator: " << type.to_string() << std::endl;
  #endif
}

std::string
SourceAST_operator_C::to_string()
{
  return std::string();
}