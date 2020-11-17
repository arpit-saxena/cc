#include "declarator.hpp"

declarator::declarator(direct_declarator *decl, pointer *p) {
  this->p = p;
  this->decl = decl;
}

identifier_declarator::identifier_declarator(std::string &&identifier) {
  this->identifier = identifier;
}

array_declarator::array_declarator(direct_declarator *decl) {
  this->decl = decl;
  raise_error("Array declarator is not supported. Ignoring some parts!");
}

param_declaration::param_declaration(declaration_specs *decl_spec,
                                     declarator *decl) {
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