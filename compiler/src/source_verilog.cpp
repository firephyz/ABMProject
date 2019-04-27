#include "source_verilog.h"

#include <string>
#include <sstream>

SourceAST_if_Verilog::SourceAST_if_Verilog(const ContextBindings& ctxt, xmlNodePtr node)
{

}

std::string
SourceAST_if_Verilog::to_source()
{
  return std::string();
}

SourceAST_assignment_Verilog::SourceAST_assignment_Verilog(const ContextBindings& ctxt, xmlNodePtr node)
{

}

std::string
SourceAST_assignment_Verilog::to_source()
{
  return std::string();
}

SourceAST_var_Verilog::SourceAST_var_Verilog(const ContextBindings& ctxt, xmlNodePtr node)
{

}

std::string
SourceAST_var_Verilog::to_source()
{
  return std::string();
}

SourceAST_constant_Verilog::SourceAST_constant_Verilog(xmlNodePtr node)
{

}

std::string
SourceAST_constant_Verilog::to_source() {
  return std::string();
}

SourceAST_operator_Verilog::SourceAST_operator_Verilog(const ContextBindings& ctxt, xmlNodePtr node)
{

}

std::string
SourceAST_operator_Verilog::to_source() {
  return std::string();
}

SourceAST_return_Verilog::SourceAST_return_Verilog(const ContextBindings& ctxt, xmlNodePtr node)
{

}

std::string
SourceAST_return_Verilog::to_source() {
  return std::string();
}

SourceAST_response_Verilog::SourceAST_response_Verilog(const ContextBindings& ctxt, xmlNodePtr node)
{

}


std::string
SourceAST_response_Verilog::to_source()
{
  return std::string();
}
