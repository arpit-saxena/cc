#include "ast.hpp"

#include <iostream>
#include <string>
#include <vector>

printer ast_node::cout;
ast_node *ast_node::base = nullptr;
llvm::LLVMContext ast_node::the_context;
llvm::IRBuilder<> ast_node::ir_builder(the_context);
std::unique_ptr<llvm::Module> ast_node::the_module =
    std::make_unique<llvm::Module>("module", the_context);

void ast_node::dump_tree() { cout << "- (ast_node)" << endl; }

void ast_node::raise_error(std::string err) {
  std::cerr << "Error: " << err << '\n';
}