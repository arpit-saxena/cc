#ifndef DECLARATION_HPP
#define DECLARATION_HPP

#include <vector>

#include "ast.hpp"
#include "expression.hpp"

class initializer_node : public ast_node {};

// TODO: Fix this. Ideally this class shouldn't exist but multiple inheritance
// is causing issues the base case not being pure virtual.
class init_assign_expr : public initializer_node {
  assign_expr *expression;

 public:
  init_assign_expr(assign_expr *expression);
};

// Not using designation for now
class initializer_lst : public initializer_node {
  std::vector<initializer_node *> inits;

 public:
  initializer_lst *add(initializer_node *init);
};

#endif /* DECLARATION_HPP */