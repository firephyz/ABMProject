#include "source_c.h"

#include <libxml/parser.h>
#include <string>
#include <sstream>

SourceC_if::SourceC_if(xmlNodePtr node)
{
}

std::string
SourceC_if::to_string()
{
  std::stringstream result;
  result << "if (" << predicate->to_string() << ") { " << then_clause->to_string() << " }";
  if(else_clause != nullptr) {
    result << " else { " << else_clause->to_string() << " }";
  }
  return result.str();
}

SourceC_assignment::SourceC_assignment(xmlNodePtr node)
{

}

std::string
SourceC_assignment::to_string()
{
  std::stringstream result;
  result << var_name << " = " << value->to_string() << ";";
  return result.str();
}
