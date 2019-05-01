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
SourceAST_if_C::to_source() const
{
  CHECK_AST_CODE_GEN_READY();

  std::stringstream result;
  result << "if " << predicate->to_source() << " {\n";
  result << util::indent(then_clause->to_source()) << "\n}\n";
  if(else_clause != nullptr) {
    result << "else {\n" << util::indent(else_clause->to_source()) << "\n}\n";
  }
  if(next != nullptr) {
    result << next->to_source();
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
  const SymbolBinding * binding = &ctxt.getBindingByName(symbol_name);
  var_binding = std::make_unique<const SourceAST_var_C>(binding);

  curNode = xmlNextElementSibling(curNode);
  if(xmlStrcmp(curNode->name, (const xmlChar *)"ask") == 0) {
    type = AssignmentValueType::CommsAnswer;
    value_answer = std::make_unique<SourceAST_ask_C>(curNode, var_binding.get());
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
    std::cout << "Assignment: " << this->to_string() << std::endl;
  #endif
}

std::string
SourceAST_assignment_C::to_source() const
{
  CHECK_AST_CODE_GEN_READY();

  std::stringstream result;
  std::string value_string = type == AssignmentValueType::Expression ? 
    value_expr->to_source() :
    (type == AssignmentValueType::CommsAnswer ?
      (value_answer.get() == nullptr ?
        "ANSWER" :
        value_answer->to_source()) :
      "NoInit");
  result << var_binding->to_source() << " = " << value_string << ";";
  return result.str();
}

std::string
SourceAST_assignment_C::to_string()
{
  std::stringstream result;

  result << "ASSIGN var=\'" << var_binding->getBinding().getName() << "\'" << std::endl;
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
SourceAST_constant_C::to_source() const
{
  CHECK_AST_CODE_GEN_READY();

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
    if(binding != nullptr) {
      std::cout << "Var: " << binding->getName() << std::endl;
    }
    else {
      std::cout << "var: NULL\n";
    }
  #endif
}

std::string
SourceAST_var_C::to_source() const
{
  CHECK_AST_CODE_GEN_READY();

  switch(binding->getScope()) {
    case SymbolBindingScope::StateLocal:
      return std::string("locals_") + binding->getScopeState().getName() + "." + binding->getName();
    case SymbolBindingScope::AgentLocal:
      return binding->getName();
    case SymbolBindingScope::Question:
      return std::string("locals->") + binding->getName();
    case SymbolBindingScope::Answer:
      return std::string("ANSWER_VAR");
    default:
      return std::string("NONE_VAR");
  }

  return std::string("ERROR");
}

std::string
SourceAST_var_C::to_string()
{
  std::stringstream result;

  result << "VAR name=\'" << binding->getName() << "\'" << std::endl;

  return result.str();
}

std::string
SourceAST_var_C::get_var_c_name() const
{
  switch(binding->getScope()) {
    case SymbolBindingScope::StateLocal:
      return std::string("locals_") + binding->getScopeState().getName() + "." + binding->getName();
      break;
    case SymbolBindingScope::AgentLocal:
      return binding->getName();
      break;
    default:
      std::cerr << "Error: Compiler currently doesn't support getting the c name for variable \'" << binding->getName() << "\' in scope \'" << SymbolBinding::scope_to_string(binding->getScope()) << "\'.\n";
      exit(-1);
      break;
  }
  return std::string();
}

SourceAST_ask_C::SourceAST_ask_C(xmlNodePtr node, const SourceAST_var * binding)
  : SourceAST_ask(binding)
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
SourceAST_ask_C::to_source() const
{
  CHECK_AST_CODE_GEN_READY();

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

std::string
SourceAST_ask_C::get_target_var_c_name() const
{
  if(target_var == nullptr) {
    std::cerr << "Compiler runtime error: Ask tag for question \'" << question->get_name() << "\' " << "is not linked.\n";
    exit(-1);
  }

  return target_var->get_var_c_name();
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
SourceAST_operator_C::to_source() const
{
  CHECK_AST_CODE_GEN_READY();

  std::stringstream result;
  std::string op_str = [&](){
    switch(type.get_type()) {
      case SourceAST_operator::OperatorTypeEnum::NoInit:
        std::cerr << "Cannot codegen AST operator node with type NoInit. Should have an actual operator.\n";
        exit(-1);
        break;
      case SourceAST_operator::OperatorTypeEnum::Add:
        return std::string("+");
      case SourceAST_operator::OperatorTypeEnum::Equal:
        return std::string("==");
      case SourceAST_operator::OperatorTypeEnum::Not:
        return std::string("!");
      case SourceAST_operator::OperatorTypeEnum::Or:
        return std::string("||");
    }
    return std::string("NONE");
  }();

  result << "(";
  if(type.get_num_args() == 1) {
    result << op_str << " " << args[0]->to_source() << ")";
  }
  else if (type.get_num_args() == 2) {
    result << args[0]->to_source() << " " << op_str << " " << args[1]->to_source() << ")";
  }
  else {
    std::cerr << "Invalid number of arguments in operator \'" << op_str << "\'\n";
    exit(-1);
  }

  return result.str();
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

// Not as simple as emitting a return statement
// The code generated will get called every time an answer is recevied
// and it will update the corresponding static question local variables.
// Returning then, is equivalent to updating the value to which the Question's
// corresponding ask tag is assigning to.
std::string
SourceAST_return_C::to_source() const
{
  CHECK_AST_CODE_GEN_READY();

  std::stringstream result;
  result << info.data.question->get_ask_tag()->get_target_var_c_name() << " = " << value->to_source() << ";";
  return result.str();
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
SourceAST_response_C::to_source() const
{
  CHECK_AST_CODE_GEN_READY();

  return std::string("responses.") + info.data.question->get_name();
}

std::string
SourceAST_response_C::to_string()
{
  std::stringstream result;

  result << "RESPONSE" << std::endl;

  return result.str();
}