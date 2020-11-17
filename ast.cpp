#include "ast.hpp"

#include <iostream>
#include <string>
#include <vector>

printer ast_node::cout;
ast_node *ast_node::base = nullptr;

void ast_node::dump_tree() { cout << "- (ast_node)" << endl; }

void ast_node::raise_error(std::string err) {
  std::cerr << "Error: " << err << '\n';
}