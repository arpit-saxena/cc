#include "declarator.hpp"

void direct_decl::dump_tree() { cout << "- (direct declarator)" << endl; }

declarator_node::declarator_node(direct_decl *decl, pointer_node *p) {
  this->p = p;
  this->decl = decl;
}

void declarator_node::dump_tree() {
  cout << "- (declarator)" << endl;
  cout.indent();
  if (p) p->dump_tree();
  decl->dump_tree();
  cout.unindent();
}

identifier_declarator::identifier_declarator(std::string &&identifier) {
  this->identifier = identifier;
}

void identifier_declarator::dump_tree() {
  cout << "- (identifier) " << identifier << endl;
}

array_declarator::array_declarator(direct_decl *decl) {
  this->decl = decl;
  raise_error("Array declarator is not supported. Ignoring some parts!");
}

void array_declarator::dump_tree() {
  cout << "-(array_declarator)" << endl;
  cout.indent();
  decl->dump_tree();
  cout.unindent();
}

param_declaration::param_declaration(declaration_specs *decl_spec,
                                     declarator_node *decl) {
  this->decl_spec = decl_spec;
  this->decl = decl;
}

void param_declaration::dump_tree() {
  cout << "- (parameter_declaration)" << endl;
  cout.indent();
  decl_spec->dump_tree();
  if (decl) decl->dump_tree();
  cout.unindent();
}

llvm::Type *param_declaration::get_type() { return decl_spec->get_type(); }

param_list *param_list::add(param_declaration *decl) {
  param_decls.push_back(decl);
  return this;
}

param_list *param_list::make_vararg() {
  is_vararg = true;
  return this;
}

void param_list::dump_tree() {
  cout << "- (parameter_list)" << endl;
  cout.indent();
  for (auto decl : param_decls) {
    decl->dump_tree();
  }
  if (is_vararg) {
    cout << "- varargs" << endl;
  }
  cout.unindent();
}

std::vector<llvm::Type *> param_list::get_types() {
  std::vector<llvm::Type *> ret(param_decls.size());
  for (int i = 0; i < param_decls.size(); i++) {
    ret[i] = param_decls[i]->get_type();
  }
  return ret;
}

function_declarator::function_declarator(direct_decl *decl,
                                         param_list *params) {
  this->decl = decl;
  this->params = params;
}

void function_declarator::old_style_error() {
  raise_error("Old style (K&R) function definition is not supported");
}

void function_declarator::dump_tree() {
  cout << "- (function_declarator)" << endl;
  cout.indent();
  decl->dump_tree();
  if (params) params->dump_tree();
  cout.unindent();
}