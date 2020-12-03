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

type_i declarator_node::get_type(type_i type) {
  if (p) type = p->get_type(type);
  return type;
}

void declarator_node::codegen(type_i type) {
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

void identifier_declarator::codegen(type_i type) {
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

void array_declarator::codegen(type_i type) {
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

type_i param_declaration::get_type() {
  type_i ret = decl_spec->get_type();
  if (decl) ret = decl->get_type(ret);
  return ret;
}

bool param_declaration::set_arg_name(llvm::Argument *arg) {
  if (!decl) return false;
  arg->setName(decl->get_identifier());
  return true;
}

void param_declaration::add_arg_table(value val) {
  if (!decl) return;
  std::string identifier = decl->get_identifier();
  auto alloca = sym_table.top_scope()->add_var(val.get_type(), identifier);
  ir_builder.CreateStore(val.llvm_val, alloca);
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

std::vector<type_i> param_list::get_types() {
  std::vector<type_i> ret(param_decls.size());
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

void param_list::add_args_table(std::vector<value> values) {
  if (values.size() != param_decls.size()) {
    raise_error("Number of values given does not match number of parameters");
  }
  int i = 0;
  for (auto val : values) {
    param_decls[i++]->add_arg_table(val);
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

llvm::Function *function_declarator::codegen_common(type_i ret_type) {
  std::vector<llvm::Type *> param_types;
  bool is_vararg = false;
  if (params) {
    auto ptypes = params->get_types();
    param_types.reserve(ptypes.size());
    std::transform(ptypes.begin(), ptypes.end(),
                   std::back_inserter(param_types),
                   [](type_i type) { return type.llvm_type; });
    is_vararg = params->is_vararg();
  }
  llvm::FunctionType *ftype =
      llvm::FunctionType::get(ret_type.llvm_type, param_types, is_vararg);

  std::string identifier = decl->get_identifier();
  llvm::Function *function = llvm::Function::Create(
      ftype, llvm::Function::ExternalLinkage, identifier, the_module.get());

  if (params) {
    params->set_arg_names(function->args());
  }
  sym_table.top_scope()->add_func(function, identifier);
  return function;
}

llvm::Function *function_declarator::codegen_def(type_i ret_type) {
  llvm::Function *func = codegen_common(ret_type);
  llvm::BasicBlock *block =
      llvm::BasicBlock::Create(the_context, "entry", func);
  ir_builder.SetInsertPoint(block);
  sym_table.change_func(func);
  sym_table.add_scope();
  if (params) {
    std::vector<type_i> types = params->get_types();
    std::vector<value> args;
    assert(types.size() == func->arg_size() &&
           "Size of parameter list should equal the number of formal "
           "arguments");
    int i = 0;
    for (auto &arg : func->args()) {
      value val(&arg, types[i++].is_signed);
      args.push_back(val);
    }
    params->add_args_table(args);
  }
  return func;
}