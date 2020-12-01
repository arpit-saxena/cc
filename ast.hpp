#ifndef AST_HPP
#define AST_HPP

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <iostream>
#include <string>
#include <vector>

#include "printer.hpp"

class ast_node {
 protected:
  static printer cout;
  static llvm::LLVMContext the_context;
  static llvm::IRBuilder<> ir_builder;
  static std::unique_ptr<llvm::Module> the_module;

 public:
  virtual void dump_tree() = 0;
  static void raise_error(std::string err);
  static ast_node *base;
};

#endif /* AST_HPP */