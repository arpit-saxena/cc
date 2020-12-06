#include "func_def.hpp"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Verifier.h>

#include <cassert>

func_def::func_def(declaration_specs *decl_specs, declarator_node *declarator,
                   compound_stmt *statement) {
  this->decl_specs = decl_specs;
  this->declarator = declarator;
  this->statement = statement;
  this->func_decl = declarator->get_func_decl();
  if (!this->func_decl) {
    raise_error("Function definition does not contain a function declarator");
  }
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
  llvm::Function *func = func_decl->codegen_def(decl_specs->get_type());

  statement->codegen();

  if (!ir_builder.GetInsertBlock()->getTerminator()) {
    if (ir_builder.GetInsertBlock()->empty()) {
      // Empty basic block. Remove it
      ir_builder.GetInsertBlock()->eraseFromParent();
    } else {
      if (!decl_specs->get_type().llvm_type->isVoidTy()) {
        print_warning(
            "No return statement in function with non-void return type");
      }
      ir_builder.CreateRetVoid();
    }
  }

  // The function scope would've been added by codegen_def of func_decl
  sym_table.pop_func_scope();

  /* for (auto &block : func->getBasicBlockList()) {
    block.print(llvm::outs());
  } */
  llvm::verifyFunction(*func);  // TODO: Print debugging info from here
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

  if (!the_module->getFunction("main")) {
    raise_error("'main' function not defined!");
  }

  if (llvm::verifyModule(*the_module, &llvm::errs())) {
    the_module->print(llvm::errs(), nullptr);
    return;
  }
}