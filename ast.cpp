#include "ast.hpp"

#include <iostream>
#include <string>
#include <vector>

std::string ast_node::dump_tree() { return "yo"; }
void ast_node::raise_error(std::string err) {
  std::cerr << "Error: " << err << '\n';
}