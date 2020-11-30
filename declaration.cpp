#include "declaration.hpp"

init_assign_expr::init_assign_expr(assign_expr *expression) {
  this->expression = expression;
}

initializer_lst *initializer_lst::add(initializer_node *init) {
  inits.push_back(init);
  return this;
}

init_decl::init_decl(declarator_node *declarator,
                     initializer_node *initializer) {
  this->declarator = declarator;
  this->initializer = initializer;
}

init_decl_list *init_decl_list::add(init_decl *declarator) {
  declarators.push_back(declarator);
  return this;
}

declaration_node::declaration_node(declaration_specs *specs,
                                   init_decl_list *decl_list) {
  this->specs = specs;
  this->decl_list = decl_list;
}