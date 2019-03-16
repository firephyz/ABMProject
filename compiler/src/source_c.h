#ifndef SOURCE_C_INCLUDED
#define SOURCE_C_INCLUDED

#include "source_ast.h"

#include <libxml/parser.h>

class SourceC_if : public SourceAST_if {
public:
  SourceC_if(xmlNodePtr node);
  std::string to_string();
};

class SourceC_assignment : public SourceAST_assignment {
public:
  SourceC_assignment(xmlNodePtr node);
  std::string to_string();
};

#endif