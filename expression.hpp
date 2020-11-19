#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <string>

#include "ast.hpp"

/*
 * There is a class hierarchy of empty classes denoting a particular kind of
 * expression And for a class actually representing that kind of expression, the
 * class is derived from that expression with "_ops" appended, meaning the
 * expression with operands.
 *
 * This is done so an expression down the hierarchy don't have to construct all
 * expressions up the hierarchy. It also makes logical sense since, for example
 * a conditional expression is an assignment expression, but "a = 5" is an
 * assignment expression and not a cast expression
 */

class expr : public ast_node {};
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

 public:
  binary_expr_ops(binary_expr *left, OP op, binary_expr *right);
  static std::string op_string(OP op);
  virtual void dump_tree() override;
};

class cast_expr : public binary_expr {};
class unary_expr : public cast_expr {};
class postfix_expr : public unary_expr {};
class primary_expr : public postfix_expr {};

class ident_expr : public primary_expr {
  std::string identifier;

 public:
  ident_expr(const char *id);
  void dump_tree() override;
};

class const_expr : public primary_expr {};

// Includes character and integer constants
// TODO: Figure out how to separate them, and parse as char/int
class int_constant_expr : public const_expr {
  std::string num_str;

 public:
  int_constant_expr(const char *id);
  void dump_tree() override;
};

class paren_expr : public primary_expr {
  expr *expression;

 public:
  paren_expr(expr *expression);
  void dump_tree() override;
};

#endif /* EXPRESSION_HPP */