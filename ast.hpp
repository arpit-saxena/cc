#pragma once

#include <iostream>
#include <string>
#include <vector>

class ast_node {
 public:
  virtual std::string dump_tree();
  static void raise_error(std::string err);
};