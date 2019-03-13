#include "c_source.h"

#include <string>
#include <sstream>

std::string
CSource_if::to_string()
{
  std::stringstream result;
  result << "if (" << predicate->to_string() << ") { " << then_clause->to_string() << " }";
  if(else_clause != nullptr) {
    result << " else { " << else_clause->to_string() << " }";
  }
  return result.str();
}

std::string
CSource_assignment::to_string()
{
  std::stringstream result;
  result << var_name << " = " << value->to_string() << ";";
  return result.str();
}