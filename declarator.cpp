#include "declarator.hpp"

declarator_node::declarator_node(direct_decl *decl, pointer_node *p) {
  this->p = p;
  this->decl = decl;
}

identifier_declarator::identifier_declarator(std::string &&identifier) {
  this->identifier = identifier;
}

array_declarator::array_declarator(direct_decl *decl) {
  this->decl = decl;
  raise_error("Array declarator is not supported. Ignoring some parts!");
}

param_declaration::param_declaration(declaration_specs *decl_spec,
                                     declarator_node *decl) {
  this->decl_spec = decl_spec;
  this->decl = decl;
}

param_list *param_list::add(param_declaration *decl) {
  param_decls.push_back(decl);
  return this;
}

param_list *param_list::make_vararg() {
  is_vararg = true;
  return this;
}

function_declarator::function_declarator(direct_decl *decl,
                                         param_list *params) {
  this->decl = decl;
  this->params = params;
}

void function_declarator::old_style_error() {
  raise_error("Old style (K&R) function definition is not supported");
}