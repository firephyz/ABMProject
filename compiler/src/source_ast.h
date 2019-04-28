// Wrappers around xml syntactic constructs that the compiler will arrange together and elaborate
// Target languages must implement the virtual functions in derived classes

#ifndef SOURCE_AST_INCLUDED
#define SOURCE_AST_INCLUDED

#include <memory>
#include <string>
#include <iostream>

#include "parser.h"

extern ParserObject parser;

class SourceAST {
public:
  std::unique_ptr<SourceAST> next;

  SourceAST() : next(nullptr) {}
  virtual ~SourceAST();
  virtual std::string to_source() = 0;
  virtual std::string to_string() = 0;
  void append_next(std::unique_ptr<SourceAST>&& next_node) {
    next = std::move(next_node);
  };

  // for debug printing
  static int print_depth; // records the tab depth of the current ast node while printing
  static int start_depth; // records the start tab depth to print the entire ast tree. Set before first call to print_tree
  static void set_start_depth(int depth) { start_depth = depth; }
  static void to_string_fall() { ++print_depth; } // increase to_string print depth
  static void to_string_rise() { -- print_depth; } // decrease to_string print depth
  static std::string to_string_prefix();
  std::string print_tree();
};

#include "compiler_types.h"

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
    Expression,
    CommsAnswer,
  };

  SourceAST_assignment()
    : type(AssignmentValueType::NoInit)
    , value_expr(nullptr)
  {}
  ~SourceAST_assignment() {}
protected:
  const SymbolBinding * binding;
  AssignmentValueType type;
  union {
    // TODO adjust Q/A Answer element. Haven't 
    std::unique_ptr<SourceAST> value_answer;
    std::unique_ptr<SourceAST> value_expr;
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

class Question;
class SourceAST_ask : public SourceAST {
protected:
  std::string question_name;
  std::shared_ptr<const Question> question;

  SourceAST_ask()
    : question(nullptr)
  {
    // we don't currently allow recursive questions
    if(parser.state != ParserState::States) {
      std::cerr << "Attempting to ask a question when parsing Questions and Answers.";
      std::cerr << "Recursive questions not currently allowed." << std::endl;
      exit(-1);
    }
  }

public:
  const std::string& getQuestionName() const { return question_name; }
  void setQuestion(std::shared_ptr<Question>& q) { question = q; }
  const std::shared_ptr<const Question>& getQuestion() { return question; }
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

class SourceAST_return : public SourceAST {
protected:
  std::unique_ptr<SourceAST> value;

  SourceAST_return()
  {
    // we don't currently allow recursive questions
    if(parser.state == ParserState::States) {
      std::cerr << "Cannot return values from state logic." << std::endl;
      exit(-1);
    }
  }
};

class SourceAST_response : public SourceAST {
protected:
  SourceAST_response()
  {
    if(parser.state != ParserState::Questions) {
      std::cerr << "The \'response\' tag is only allowed in Question bodies. It denotes\
        the value of the answer that is given to the agent asking the question." << std::endl;
      exit(-1);
    }
  }
};

#endif
