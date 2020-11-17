#include "func_def.hpp"

func_def::func_def(declaration_specs *decl_specs, declarator_node *declarator,
                   compound_stmt *statement) {
  this->decl_specs = decl_specs;
  this->declarator = declarator;
  this->statement = statement;
}

void func_def::old_style_error() {
  raise_error("Old-style (K&R) function definition is not supported.");
}

void func_def::dump_tree() {
  cout << "- (function_definition)" << endl;
  cout.indent();
  decl_specs->dump_tree();
  declarator->dump_tree();
  statement->dump_tree();
  cout.unindent();
}

trans_unit *trans_unit::add(external_decl *decl) {
  external_decls.push_back(decl);
  return this;
}

void trans_unit::dump_tree() {
  cout << "- (translation_unit)" << endl;
  cout.indent();
  for (auto &decl : external_decls) {
    decl->dump_tree();
  }
  cout.unindent();
}