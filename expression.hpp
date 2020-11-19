#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <string>

#include "ast.hpp"

class expr;

class primary_expr : public ast_node {};

class ident_expr : public primary_expr {
  std::string identifier;

 public:
  ident_expr(const char *id);
};

class const_expr : public primary_expr {};

// Includes character and integer constants
// TODO: Figure out how to separate them, and parse as char/int
class int_constant_expr : public const_expr {
  std::string num_str;

 public:
  int_constant_expr(const char *id);
};

class paren_expr : public primary_expr {
  expr *expression;

 public:
  paren_expr(expr *expression);
};

class postfix_expr : public primary_expr {};
class unary_expr : public postfix_expr {};
class cast_expr : public unary_expr {};

class mult_expr : public cast_expr {
  mult_expr *left_expr;
  char op;
  cast_expr *right_expr;

 public:
  mult_expr(mult_expr *left, char op, cast_expr *right);
};

class add_expr : public mult_expr {
  add_expr *left_expr;
  char op;
  mult_expr *right_expr;

 public:
  add_expr(add_expr *left, char op, mult_expr *right);
};

class shift_expr : public add_expr {
 public:
  enum OP { SHIFT_LEFT, SHIFT_RIGHT };

 private:
  shift_expr *left_expr;
  OP op;
  add_expr *right_expr;

 public:
  shift_expr(shift_expr *left, OP op, add_expr *right);
};

class rel_expr : public shift_expr {
 public:
  enum OP {
    LT,  // '<' : less than
    GT,  // '>' : greather than
    LE,  // '<=' : less than or equal to
    GE   // '>=' : greater than or equal to
  };

 private:
  rel_expr *left_expr;
  OP op;
  shift_expr *right_expr;

 public:
  rel_expr(rel_expr *left, OP op, shift_expr *right);
};

class eq_expr : public rel_expr {};

class and_expr : public eq_expr {
  and_expr *left_expr;
  eq_expr *right_expr;

 public:
  and_expr(and_expr *left, eq_expr *right);
};

class xor_expr : public and_expr {};

class or_expr : public xor_expr {
  or_expr *left_expr;
  xor_expr *right_expr;

 public:
  or_expr(or_expr *left, xor_expr *right);
};

class logic_and_expr : public or_expr {
  logic_and_expr *left_expr;
  or_expr *right_expr;

 public:
  logic_and_expr(logic_and_expr *left, or_expr *right);
};

class logic_or_expr : public logic_and_expr {
  logic_or_expr *left_expr;
  logic_and_expr *right_expr;

 public:
  logic_or_expr(logic_or_expr *left, logic_and_expr *right);
};

class cond_expr : public logic_or_expr {};
class assign_expr : public cond_expr {};
class expr : public assign_expr {};

#endif /* EXPRESSION_HPP */