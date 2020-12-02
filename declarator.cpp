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

std::string declarator_node::get_identifier() { return decl->get_identifier(); }

llvm::Type *declarator_node::get_type(llvm::Type *type) {
  if (p) type = p->get_type(type);
  return type;
}

void declarator_node::codegen(llvm::Type *type) {
  if (p) type = p->get_type(type);
  decl->codegen(type);
}

identifier_declarator::identifier_declarator(std::string &&identifier) {
  this->identifier = identifier;
}

void identifier_declarator::dump_tree() {
  cout << "- (identifier) " << identifier << endl;
}

std::string identifier_declarator::get_identifier() { return identifier; }

void identifier_declarator::codegen(llvm::Type *type) {
  if (sym_table.check_top_scope(identifier)) {
    print_warning("Duplicate variable declaration. Ignoring the new one");
    return;
  }

  sym_table.top_scope()->add_var(type, identifier);
}

array_declarator::array_declarator(direct_decl *decl) {
  this->decl = decl;
  print_warning("Array declarator is not supported. Ignoring some parts!");
}

void array_declarator::dump_tree() {
  cout << "-(array_declarator)" << endl;
  cout.indent();
  decl->dump_tree();
  cout.unindent();
}

void array_declarator::codegen(llvm::Type *type) {
  raise_error("Arrays are not supported yet!");
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

llvm::Type *param_declaration::get_type() {
  llvm::Type *ret = decl_spec->get_type();
  if (decl) ret = decl->get_type(ret);
  return ret;
}

bool param_declaration::set_arg_name(llvm::Argument *arg) {
  if (!decl) return false;
  arg->setName(decl->get_identifier());
  return true;
}

param_list *param_list::add(param_declaration *decl) {
  param_decls.push_back(decl);
  return this;
}

param_list *param_list::make_vararg() {
  varargs = true;
  return this;
}

void param_list::dump_tree() {
  cout << "- (parameter_list)" << endl;
  cout.indent();
  for (auto decl : param_decls) {
    decl->dump_tree();
  }
  if (varargs) {
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

bool param_list::is_vararg() { return varargs; }

void param_list::set_arg_names(llvm::iterator_range<llvm::Argument *> args) {
  int i = 0;
  for (auto &arg : args) {
    param_decls[i++]->set_arg_name(&arg);
  }
}

function_declarator::function_declarator(direct_decl *decl,
                                         param_list *params) {
  this->decl = decl;
  this->params = params;
}

void function_declarator::old_style_error() {
  print_warning("Old style (K&R) function definition is not supported");
}

void function_declarator::dump_tree() {
  cout << "- (function_declarator)" << endl;
  cout.indent();
  decl->dump_tree();
  if (params) params->dump_tree();
  cout.unindent();
}

std::string function_declarator::get_identifier() {
  return decl->get_identifier();
}

void function_declarator::codegen(llvm::Type *ret_type) {
  std::vector<llvm::Type *> param_types;
  if (params) {
    param_types = params->get_types();
  }
  llvm::FunctionType *ftype =
      llvm::FunctionType::get(ret_type, param_types, params->is_vararg());

  std::string identifier = decl->get_identifier();
  llvm::Function *function = llvm::Function::Create(
      ftype, llvm::Function::ExternalLinkage, identifier, the_module.get());

  if (params) {
    params->set_arg_names(function->args());
  }
  sym_table.top_scope()->add_func(function, identifier);
}