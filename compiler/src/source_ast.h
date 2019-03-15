// Wrappers around xml syntactic constructs that the compiler will arrange together and elaborate
// Target languages must implement the virtual functions in derived classes

#ifndef SOURCE_AST_INCLUDED
#define SOURCE_AST_INCLUDED

#include <memory>
#include <string>

class SourceAST {
public:
  virtual std::string to_string() = 0;
};

class SourceAST_if : public SourceAST {
protected:
  std::unique_ptr<SourceAST> predicate;
  std::unique_ptr<SourceAST> then_clause;
  std::unique_ptr<SourceAST> else_clause;
};

class SourceAST_assignment : public SourceAST {
protected:
  const std::string var_name;
  std::unique_ptr<SourceAST> value;
};

#endif