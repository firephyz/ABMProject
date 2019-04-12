// Wrappers around xml syntactic constructs that the compiler will arrange together and elaborate
// Target languages must implement the virtual functions in derived classes

#ifndef SOURCE_AST_INCLUDED
#define SOURCE_AST_INCLUDED

// Uncomment to show the ast nodes being generated in stdout
#define VERBOSE_AST_GEN

#include "compiler_types.h"

#include <memory>
#include <string>

class SourceAST {
public:
  std::unique_ptr<SourceAST> next;

  virtual ~SourceAST() {
    if(next.get() != nullptr) {
      delete next.get();
    }
  }
  virtual std::string to_string() = 0;
  void append_next(std::unique_ptr<SourceAST>&& next_node) {
    next = std::move(next_node);
  };
};

class SourceAST_if : public SourceAST {
public:
  ~SourceAST_if() = default;
protected:
  std::unique_ptr<SourceAST> predicate;
  std::unique_ptr<SourceAST> then_clause;
  std::unique_ptr<SourceAST> else_clause;
};

class SourceAST_assignment : public SourceAST {
public:
  enum class AssignmentValueType {
    NoInit,
    Constant,
    CommsAnswer,
  };

  SourceAST_assignment()
    : type(AssignmentValueType::NoInit)
  {}
  ~SourceAST_assignment() {}
protected:
  const SymbolBinding * binding;
  AssignmentValueType type;
  union {
    // TODO adjust Q/A Answer element. Haven't 
    std::unique_ptr<SourceAST> value_answer;
    std::unique_ptr<SourceAST> value_constant;
  };
};

class SourceAST_constant : public SourceAST {
public:
  enum class ConstantType {
    NoInit,
    Integer,
    Real,
    Bool,
  };

  SourceAST_constant()
    : type(ConstantType::NoInit)
  {}

protected:
  ConstantType type;
  std::string value;
};

class SourceAST_var : public SourceAST {
protected:
  const SymbolBinding * binding;
};

class SourceAST_ask : public SourceAST {
protected:
  // question
};

class SourceAST_operator : public SourceAST {
public:
  enum class OperatorTypeEnum {
    NoInit,
    Add,
    Equal,
    Not,
    Or,
  };
  class OperatorType {
  protected:
    OperatorTypeEnum type;
    int num_args;
  public:
    OperatorType(OperatorTypeEnum type, int num_args)
      : type(type)
      , num_args(num_args)
    {}

    bool isType(const OperatorTypeEnum& other) { return type == other; }
    std::string to_string() {
      switch(type) {
        case OperatorTypeEnum::NoInit:
          return std::string("NoInit");
        case OperatorTypeEnum::Add:
          return std::string("Add");
        case OperatorTypeEnum::Equal:
          return std::string("Equal");
        case OperatorTypeEnum::Not:
          return std::string("Not");
        case OperatorTypeEnum::Or:
          return std::string("Or");
      }

      // shouldn't get here
      return std::string();
    }
  };

  SourceAST_operator()
    : type(OperatorType(OperatorTypeEnum::NoInit, -1))
  {}

protected:
  OperatorType type;
  std::vector<std::unique_ptr<SourceAST>> args;
};

#endif