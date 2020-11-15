#include "ast.hpp"

#include <iostream>
#include <string>
#include <vector>

std::string ast_node::dump_tree() { return "yo"; }
void ast_node::raise_error(std::string err) {
  std::cerr << "Error: " << err << '\n';
}

decl_specifiers_node *decl_specifiers_node::add_storage_spec(
    storage_specifier spec) {
  if (storage != storage_specifier::UNSET) {
    raise_error("Cannot have multiple storage specifiers");
    return this;
  }

  storage = spec;
  return this;
}

decl_specifiers_node *decl_specifiers_node::add_type_spec(
    basic_type_spec spec) {
  if (type != basic_type_spec::UNSET) {
    raise_error("Not implemented: multiple type specifiers");
    return this;
  }

  type = spec;
  return this;
}

param_decl_node::param_decl_node(ast_node *specifiers, ast_node *decl) {
  this->specifiers = dynamic_cast<decl_specifiers_node *>(specifiers);
  this->decl = dynamic_cast<declarator_node *>(decl);
}

param_list_node::param_list_node(param_decl_node *decl) { add_decl(decl); }

param_list_node *param_list_node::add_decl(param_decl_node *decl) {
  param_decls.push_back(dynamic_cast<param_decl_node *>(decl));
  // std::cout << param_decls.size() << '\n';
  return this;
}