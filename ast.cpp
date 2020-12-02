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

void ast_node::print_warning(std::string err) {
  std::cerr << "Warning: " << err << '\n';
}

void ast_node::raise_error(std::string err) {
  std::cerr << "Error: " << err << '\n';
  std::exit(1);
}