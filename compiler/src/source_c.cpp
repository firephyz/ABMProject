#include "source_c.h"

#include <libxml/parser.h>
#include <string>
#include <sstream>
#include <iostream>

SourceAST_if_C::SourceAST_if_C(xmlNodePtr node)
{
  xmlNodePtr child = xmlFirstElementChild(node);   

  while (child != NULL) { 
    if (xmlStrcmp(child->name, (const xmlChar*)"predicate") == 0) { 
      //parse_logic()
    }
    else if(xmlStrcmp(child->name, (const xmlChar*)"then") == 0) {

    }
    else if(xmlStrcmp(child->name, (const xmlChar*)"else") == 0) {

    }
    else {
      std::cerr << "Invalid xml tag \'" << child->name << "\' under \'<if>\' tag.\n";
      exit(-1);
    }

    child = xmlNextElementSibling(child);
  }
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

SourceAST_assignment_C::SourceAST_assignment_C(xmlNodePtr node)
{

}

std::string
SourceAST_assignment_C::to_string()
{
  std::stringstream result;
  result << var_name << " = " << value->to_string() << ";";
  return result.str();
}
