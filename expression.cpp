#include "expression.hpp"

#include <string>

ident_expr::ident_expr(const char *id) { identifier = std::string(id); }

int_constant_expr::int_constant_expr(const char *num_str) {
  this->num_str = num_str;
}

paren_expr::paren_expr(expr *expression) { this->expression = expression; }

mult_expr::mult_expr(mult_expr *left, char op, cast_expr *right) {
  this->left_expr = left;
  this->op = op;
  this->right_expr = right;
}

add_expr::add_expr(add_expr *left, char op, mult_expr *right) {
  this->left_expr = left;
  this->op = op;
  this->right_expr = right;
}

shift_expr::shift_expr(shift_expr *left, shift_expr::OP op, add_expr *right) {
  this->left_expr = left;
  this->op = op;
  this->right_expr = right;
}

rel_expr::rel_expr(rel_expr *left, rel_expr::OP op, shift_expr *right) {
  this->left_expr = left;
  this->op = op;
  this->right_expr = right;
}

and_expr::and_expr(and_expr *left, eq_expr *right) {
  this->left_expr = left;
  this->right_expr = right;
}

or_expr::or_expr(or_expr *left, xor_expr *right) {
  this->left_expr = left;
  this->right_expr = right;
}

logic_and_expr::logic_and_expr(logic_and_expr *left, or_expr *right) {
  this->left_expr = left;
  this->right_expr = right;
}

logic_or_expr::logic_or_expr(logic_or_expr *left, logic_and_expr *right) {
  this->left_expr = left;
  this->right_expr = right;
}