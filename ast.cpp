#include "ast.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "func_def.hpp"

printer ast_node::cout;
trans_unit *ast_node::base = nullptr;
llvm::LLVMContext ast_node::the_context;
llvm::IRBuilder<> ast_node::ir_builder(the_context);
std::unique_ptr<llvm::Module> ast_node::the_module =
    std::make_unique<llvm::Module>("module", the_context);
symbol_table ast_node::sym_table(ir_builder);

void ast_node::print_warning(std::string err) {
  std::cerr << "Warning: " << err << '\n';
}

void ast_node::raise_error [[noreturn]] (std::string err) {
  std::cerr << "Error: " << err << '\n';
  std::exit(1);
}

value ast_node::create_load(value val, std::string identifier) {
  if (llvm::isa<llvm::Function>(val.llvm_val) ||
      llvm::isa<llvm::GlobalVariable>(val.llvm_val)) {
    return val;  // Don't create a load for functions and global variables
  }

  val.llvm_val = ir_builder.CreateLoad(val.llvm_val, identifier);
  return val;
  llvm::Value *llvm_val = val.llvm_val;
}