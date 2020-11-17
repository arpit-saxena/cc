#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <string>
#include <vector>

#include "printer.hpp"

class ast_node {
 protected:
  static printer cout;

 public:
  virtual void dump_tree();
  static void raise_error(std::string err);
  static ast_node *base;
};

#endif /* AST_HPP */