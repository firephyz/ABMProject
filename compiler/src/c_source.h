// Wrappers around C syntactic constructs that the compiler will arrange together and elaborate

#ifndef C_SOURCE_INCLUDED
#define C_SOURCE_INCLUDED

#include <memory>
#include <string>

class CSource {
public:
  virtual std::string to_string() = 0;
};

class CSource_if : public CSource {
  std::unique_ptr<CSource> predicate;
  std::unique_ptr<CSource> then_clause;
  std::unique_ptr<CSource> else_clause;
public:
  CSource_if(std::unique_ptr<CSource>& predicate, std::unique_ptr<CSource>& then_clause, std::unique_ptr<CSource>& else_clause)
    : predicate(std::move(predicate))
    , then_clause(std::move(then_clause))
    , else_clause(std::move(else_clause))
  {}
  std::string to_string();
};

class CSource_assignment : public CSource {
  const std::string var_name;
  std::unique_ptr<CSource> value;
public:
  CSource_assignment(const std::string var_name, std::unique_ptr<CSource>& value)
    : var_name(var_name)
    , value(std::move(value))
  {}
  std::string to_string();
};

#endif