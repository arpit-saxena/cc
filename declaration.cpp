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

value init_assign_expr::codegen() { return expression->codegen(); }

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

value initializer_lst::codegen() {
  raise_error("Code generation not defined for initializer lists");
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

value init_decl::codegen(type_i type) {
  value decl = declarator->codegen(type);
  if (initializer) {
    value init_val = initializer->codegen();
    if (auto func = llvm::dyn_cast<llvm::Function>(decl.llvm_val)) {
      raise_error("Initializer not allowed for functions");
    } else if (auto global =
                   llvm::dyn_cast<llvm::GlobalVariable>(decl.llvm_val)) {
      if (auto const_val = llvm::dyn_cast<llvm::Constant>(init_val.llvm_val)) {
        global->setInitializer(const_val);
        return decl;
      } else {
        raise_error(
            "Non compile-time evaluatable expressions can't be used as "
            "initializers for global variables. Note more expressions will "
            "eventually be allowed as optimizations are done");
      }
    }

    ir_builder.CreateStore(init_val.llvm_val, decl.llvm_val);
  }
  return decl;
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

std::vector<value> init_decl_list::codegen(type_i type) {
  std::vector<value> vals;
  vals.reserve(declarators.size());
  std::transform(declarators.begin(), declarators.end(),
                 std::back_inserter(vals),
                 [type](init_decl *decl) { return decl->codegen(type); });
  return vals;
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

void declaration_node::codegen() {
  if (!decl_list) {
    if (allow_no_declarator) return;
    raise_error("No declarator present in declaration!");
  }

  decl_list->codegen(specs->get_type());
}