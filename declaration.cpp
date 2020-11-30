#include "declaration.hpp"

init_assign_expr::init_assign_expr(assign_expr *expression) {
  this->expression = expression;
}

initializer_lst *initializer_lst::add(initializer_node *init) {
  inits.push_back(init);
  return this;
}