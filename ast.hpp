#ifndef AST_HPP
#define AST_HPP

#include <llvm/IR/Module.h>

#include <iostream>
#include <string>
#include <vector>

#include "printer.hpp"

class ast_node {
 protected:
  static printer cout;
  static llvm::LLVMContext the_context;
  static std::unique_ptr<llvm::Module> the_module;

 public:
  virtual void dump_tree();
  static void raise_error(std::string err);
  static ast_node *base;
};

#endif /* AST_HPP */