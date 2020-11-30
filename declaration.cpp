#include "declaration.hpp"

void initializer_node::dump_tree() { cout << "- (initializer)" << endl; }

init_assign_expr::init_assign_expr(assign_expr *expression) {
  this->expression = expression;
}

void init_assign_expr::dump_tree() {
  cout << "- (initializer)" << endl;
  cout.indent();
  expression->dump_tree();
  cout.unindent();
}

initializer_lst *initializer_lst::add(initializer_node *init) {
  inits.push_back(init);
  return this;
}

void initializer_lst::dump_tree() {
  cout << "- (initializer_list)" << endl;
  cout.indent();
  for (auto &init : inits) {
    init->dump_tree();
  }
  cout.unindent();
}

init_decl::init_decl(declarator_node *declarator,
                     initializer_node *initializer) {
  this->declarator = declarator;
  this->initializer = initializer;
}

void init_decl::dump_tree() {
  cout << "- (init_declarator)" << endl;
  cout.indent();
  declarator->dump_tree();
  if (initializer) initializer->dump_tree();
  cout.unindent();
}

init_decl_list *init_decl_list::add(init_decl *declarator) {
  declarators.push_back(declarator);
  return this;
}

void init_decl_list::dump_tree() {
  cout << "- (init_declarator_list)" << endl;
  cout.indent();
  for (auto &decl : declarators) {
    decl->dump_tree();
  }
  cout.unindent();
}

declaration_node::declaration_node(declaration_specs *specs,
                                   init_decl_list *decl_list) {
  this->specs = specs;
  this->decl_list = decl_list;
}

void declaration_node::dump_tree() {
  cout << "- (declaration)" << endl;
  cout.indent();
  specs->dump_tree();
  if (decl_list) decl_list->dump_tree();
  cout.unindent();
}