// Wrappers around xml syntactic constructs that the compiler will arrange together and elaborate
// Target languages must implement the virtual functions in derived classes

#ifndef SOURCE_AST_INCLUDED
#define SOURCE_AST_INCLUDED

#include <memory>
#include <string>
#include <iostream>

#include "parser.h"
#include "compiler_types.h"
#include "agent_form.h"

// Ensures that the programmer calls the correct code-gen function
#define CHECK_AST_CODE_GEN_READY()\
do {\
  if(!info.is_setup) {\
    std::cerr << "Compiler runtime error when starting AST code-gen. You must use SourceAST::to_source_start()\
  to start code-gen, not the derived to_source method.\n";\
    exit(-1);\
  }\
} while(false)

extern ParserObject parser;

enum ASTNodeType {
  Node_if,
  Node_assignment,
  Node_constant,
  Node_var,
  Node_ask,
  Node_operator,
  Node_return,
  Node_response,
};

class Question;
class Answer;
class StateInstance;

enum class SourceASTInfoType {
  None,
  Question,
  Answer,
  StateInstance,
};

struct sourceast_info_t {
  union {
    const Question * question;
    const Answer * answer;
    const StateInstance * state;
  } data;
  bool is_setup = false;
  SourceASTInfoType type = SourceASTInfoType::None;
};

class SourceAST {
public:
  // Used during AST code-gen to get information related to what its generating code for.
  // Certainly not designed for parallel AST code-gen
  static struct sourceast_info_t info;

  std::unique_ptr<SourceAST> next;

  SourceAST() : next(nullptr) {}
  virtual ~SourceAST();
  virtual std::string to_source() const = 0;
  std::string to_source_start(SourceASTInfoType type, void * ptr);
  virtual std::string to_string() = 0;
  virtual ASTNodeType get_type() const = 0;
  virtual std::string gen_c_default_value() const {
    std::cerr << "Cannot call \'gen_c_default_value\' function on a SourceAST node of type\'" << this->type_to_string() << "\'" << std::endl;
    exit(-1);
    return std::string();
  }
  virtual std::string gen_declaration() const {
    std::cerr << "Cannot call \'gen_declaration\' function on a SourceAST node of type\'" << this->type_to_string() << "\'" << std::endl;
    exit(-1);
    return std::string();
  }
  virtual std::string get_target_var_c_name() const {
    std::cerr << "Cannot call \'get_target_var_c_name\' function on a SourceAST node of type \'" << this->type_to_string() << "\'" << std::endl;
    exit(-1);
  }
  virtual std::string get_var_c_name() const {
    std::cerr << "Cannot call \'get_var_c_name\' function on a SourceAST node of type \'" << this->type_to_string() << "\'" << std::endl;
    exit(-1);
  }
  virtual const SymbolBinding& getBinding() const {
    std::cerr << "Cannot call \'getBinding\' function on a SourceAST node of type \'" << this->type_to_string() << "\'" << std::endl;
    exit(-1);
  }
  std::string type_to_string() const {
    switch(this->get_type()) {
      case ASTNodeType::Node_if:
        return "Node_if";
      case ASTNodeType::Node_assignment:
        return "Node_assignment";
      case ASTNodeType::Node_constant:
        return "Node_constant";
      case ASTNodeType::Node_var:
        return "Node_var";
      case ASTNodeType::Node_ask:
        return "Node_ask";
      case ASTNodeType::Node_operator:
        return "Node_operator";
      case ASTNodeType::Node_return:
        return "Node_return";
      case ASTNodeType::Node_response:
        return "Node_response";
    }
    return std::string();
  }
  virtual std::string gen_type() const {
    std::cerr << "Cannot call \'gen_type\' on a \'" << get_type() << "\' node." << std::endl;
    exit(-1);
  }
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
  ASTNodeType get_type() const { return ASTNodeType::Node_if; }
protected:
  std::unique_ptr<SourceAST> predicate;
  std::unique_ptr<SourceAST> then_clause;
  std::unique_ptr<SourceAST> else_clause;
};

class SymbolBinding;
class SourceAST_var;
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
  ASTNodeType get_type() const { return ASTNodeType::Node_assignment; }
protected:
  //const SymbolBinding * binding;
  std::unique_ptr<const SourceAST_var> var_binding;
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
    State,
  };

  SourceAST_constant()
    : type(ConstantType::NoInit)
  {}
  ASTNodeType get_type() const { return ASTNodeType::Node_constant; }
  const std::string& getValue() const { return value; }
  std::string gen_declaration() const;
  std::string gen_c_default_value() const;
  std::string gen_type() const {
    switch(type) {
      case ConstantType::NoInit:
        return std::string("ERROR");
      case ConstantType::Integer:
        return std::string("int");
      case ConstantType::Real:
        return std::string("double");
      case ConstantType::Bool:
        return std::string("bool");
      case ConstantType::State:
        return std::string("AgentState");
    }
    return std::string();
  }

protected:
  ConstantType type;
  std::string value;
};

class SourceAST_var : public SourceAST {
public:
  SourceAST_var() : binding(nullptr) {}
  SourceAST_var(const SymbolBinding * binding) : binding(binding) {}
  ASTNodeType get_type() const { return ASTNodeType::Node_var; }
  std::string gen_declaration() const;
  std::string gen_type() const;
  std::string gen_c_default_value() const;
  const SymbolBinding& getBinding() const { return *binding; }
protected:
  const SymbolBinding * binding;
};

class Question;
class SourceAST_ask : public SourceAST {
protected:
  std::string question_name;
  std::shared_ptr<const Question> question;
  const SourceAST * target_var = nullptr;

  SourceAST_ask(const SourceAST_var * var)
    : question(nullptr)
    , target_var(var)
  {
    // we don't currently allow recursive questions
    if(parser.state != ParserState::States) {
      std::cerr << "Attempting to ask a question when parsing Questions and Answers.";
      std::cerr << "Recursive questions not currently allowed." << std::endl;
      exit(-1);
    }
  }
  ASTNodeType get_type() const { return ASTNodeType::Node_ask; }

public:
  const std::string& getQuestionName() const { return question_name; }
  void setQuestion(std::shared_ptr<Question>& q) { question = q; }
  const std::shared_ptr<const Question>& getQuestion() { return question; }
  void set_target_var(const SourceAST& var) { this->target_var = &var; }
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
    int get_num_args() const { return num_args; }
    OperatorTypeEnum get_type() const { return type; }
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
  ASTNodeType get_type() const { return ASTNodeType::Node_operator; }

protected:
  OperatorType type;
  std::vector<std::unique_ptr<SourceAST>> args;
};

class SourceAST_return : public SourceAST {
public:
  SourceAST_return()
  {
    if(parser.state == ParserState::States) {
      std::cerr << "Cannot return values from state logic." << std::endl;
      exit(-1);
    }
  }
  ASTNodeType get_type() const { return ASTNodeType::Node_return; }
protected:
  std::unique_ptr<SourceAST> value;
};

class SourceAST_response : public SourceAST {
public:
  SourceAST_response()
  {
    if(parser.state != ParserState::Questions) {
      std::cerr << "The \'response\' tag is only allowed in Question bodies. It denotes\
        the value of the answer that is given to the agent asking the question." << std::endl;
      exit(-1);
    }
  }
  ASTNodeType get_type() const { return ASTNodeType::Node_response; }
protected:
};

#endif
