#include "func_def.hpp"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>

#include <cassert>

func_def::func_def(declaration_specs *decl_specs, declarator_node *declarator,
                   compound_stmt *statement) {
  this->decl_specs = decl_specs;
  this->declarator = declarator;
  this->statement = statement;
}

void func_def::old_style_error() {
  print_warning("Old-style (K&R) function definition is not supported.");
}

void func_def::dump_tree() {
  cout << "- (function_definition)" << endl;
  cout.indent();
  decl_specs->dump_tree();
  declarator->dump_tree();
  statement->dump_tree();
  cout.unindent();
}

void func_def::codegen() {
  declarator->codegen(decl_specs->get_type());
  std::string identifier = declarator->get_identifier();
  scope::var_opt func_opt = sym_table.get_var(identifier);
  assert(func_opt.has_value() &&
         std::holds_alternative<llvm::Function *>(func_opt.value()));
  llvm::Function *func = std::get<llvm::Function *>(func_opt.value());
  llvm::BasicBlock *block =
      llvm::BasicBlock::Create(the_context, "entry", func);
  ir_builder.SetInsertPoint(block);
  statement->codegen();
  if (decl_specs->get_type()->isVoidTy()) {
    ir_builder.CreateRetVoid();
  }
  llvm::verifyFunction(*func);
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

void trans_unit::codegen() {
  for (auto decl : external_decls) {
    decl->codegen();
  }
  if (llvm::verifyModule(*the_module, &llvm::errs())) {
    return;
  }
  the_module->print(llvm::errs(), nullptr);
}