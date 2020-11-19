#include "expression.hpp"

#include <string>

binary_expr_ops::binary_expr_ops(binary_expr *left, OP op, binary_expr *right) {
  this->left_expr = left;
  this->op = op;
  this->right_expr = right;
}

ident_expr::ident_expr(const char *id) { identifier = std::string(id); }

int_constant_expr::int_constant_expr(const char *num_str) {
  this->num_str = num_str;
}

paren_expr::paren_expr(expr *expression) { this->expression = expression; }