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

llvm::Function *declarator_node::gen_function(declaration_specs *specs) {
  llvm::Type *ret_type = specs->get_type();
  if (p) {
    ret_type = p->get_type(ret_type);
  }
  auto fn_decl = dynamic_cast<function_declarator *>(decl);
  if (!fn_decl) {
    raise_error("direct declarator is not a valid function declarator");
    return nullptr;
  }
  return fn_decl->get_function(ret_type);
}

identifier_declarator::identifier_declarator(std::string &&identifier) {
  this->identifier = identifier;
}

void identifier_declarator::dump_tree() {
  cout << "- (identifier) " << identifier << endl;
}

std::string identifier_declarator::get_identifier() { return identifier; }

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
  raise_error("Old style (K&R) function definition is not supported");
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

llvm::Function *function_declarator::get_function(llvm::Type *ret_type) {
  std::vector<llvm::Type *> param_types;
  if (params) {
    param_types = params->get_types();
  }
  llvm::FunctionType *ftype =
      llvm::FunctionType::get(ret_type, param_types, params->is_vararg());

  llvm::Function *function =
      llvm::Function::Create(ftype, llvm::Function::ExternalLinkage,
                             decl->get_identifier(), the_module.get());

  if (params) {
    params->set_arg_names(function->args());
  }

  return function;
}