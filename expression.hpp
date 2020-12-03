#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <llvm/IR/Value.h>

#include <string>
#include <utility>

#include "ast.hpp"

/*
 * The frontend needs to maintain more data for a value than a llvm::Value holds
 * such as the signedness, since it determines which instructions to emit.
 * This struct is a wrapper of llvm::Value and any other extra data that is to
 * be maintained
 */
struct value {
  llvm::Value *llvm_val;
  bool is_signed = false;
};

/*
 * There is a class hierarchy of empty classes denoting a particular kind of
 * expression And for a class actually representing that kind of expression, the
 * class is derived from that expression with "_ops" appended, meaning the
 * expression with operands.
 *
 * This is done so an expression down the hierarchy don't have to construct all
 * expressions up the hierarchy. It also makes logical sense since, for example
 * a conditional expression is an assignment expression, but "a = 5" is an
 * assignment expression and not a conditional expression
 */

class expr : public ast_node {
 public:
  virtual value codegen() {
    raise_error("codegen not implemented for this expression!");
  };  // TODO: Make this pure virtual
};

class assign_expr : public expr {};
class cond_expr : public assign_expr {};

class binary_expr : public cond_expr {};

// Rest of the expressions have been combined into one class
// If need be later, this can be broken down into the proper hierarchy
class binary_expr_ops : public binary_expr {
 public:
  enum OP {
    MULT,         // '*'
    DIV,          // '/'
    MOD,          // '%'
    PLUS,         // '+'
    MINUS,        // '-'
    SHIFT_LEFT,   // '<<'
    SHIFT_RIGHT,  // '>>'
    LT,           // '<'
    GT,           // '>'
    LE,           // '<='
    GE,           // '>='
    EQ,           // '=='
    NE,           // '!='
    BIT_AND,      // '&'
    BIT_XOR,      // '^'
    BIT_OR,       // '|'
    AND,          // '&&'
    OR,           // '||'
  };

 private:
  binary_expr *left_expr;
  OP op;
  binary_expr *right_expr;
  static llvm::Instruction::BinaryOps get_arith_op(value lhs, value rhs, OP op);
  static llvm::CmpInst::Predicate get_cmp_pred(value lhs, value rhs, OP op);
  static void gen_common_type(value lhs, value rhs);

 public:
  binary_expr_ops(binary_expr *left, OP op, binary_expr *right);
  static std::string op_string(OP op);
  virtual void dump_tree() override;
  value codegen() override;
};

class cast_expr : public binary_expr {};
class unary_expr : public cast_expr {};

class arg_expr_list : public ast_node {
  std::vector<assign_expr *> exprs;

 public:
  arg_expr_list *add(assign_expr *expr);
  void dump_tree() override;
};

class postfix_expr : public unary_expr {};

class func_call : public postfix_expr {
  postfix_expr *func_expr;
  arg_expr_list *arg_list;  // Optional

 public:
  func_call(postfix_expr *func_expr, arg_expr_list *expr_list = nullptr);
  void dump_tree() override;
};

class primary_expr : public postfix_expr {};

class ident_expr : public primary_expr {
  std::string identifier;

 public:
  ident_expr(const char *id);
  void dump_tree() override;
};

class paren_expr : public primary_expr {
  expr *expression;

 public:
  paren_expr(expr *expression);
  void dump_tree() override;
};

class const_expr : public primary_expr {
  llvm::Constant *data;
  std::string str;

 public:
  const_expr(llvm::Constant *data, std::string str);
  static std::pair<llvm::APInt, llvm::Type *> get_int(std::string str);
  // includes character literals
  static const_expr *new_int_expr(const char *str);
  void dump_tree() override;
};

class string_expr : public primary_expr {
  const std::string str;

 public:
  string_expr(const char *str);
  void dump_tree() override;
};

#endif /* EXPRESSION_HPP */
