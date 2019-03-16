#ifndef SOURCE_C_INCLUDED
#define SOURCE_C_INCLUDED

#include "source_ast.h"

#include <libxml/parser.h>

class SourceAST_if_C : public SourceAST_if {
public:
  SourceAST_if_C(xmlNodePtr node);
  std::string to_string();
};

class SourceAST_assignment_C : public SourceAST_assignment {
public:
  SourceAST_assignment_C(xmlNodePtr node);
  std::string to_string();
};

#endif