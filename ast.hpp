#ifndef AST_HPP
#define AST_HPP

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <iostream>
#include <string>
#include <vector>

#include "printer.hpp"
#include "symbol_table.hpp"

class trans_unit;  // Forward declaration

class ast_node {
 protected:
  static printer cout;
  static llvm::LLVMContext the_context;
  static llvm::IRBuilder<> ir_builder;
  static std::unique_ptr<llvm::Module> the_module;
  static symbol_table sym_table;

 public:
  virtual void dump_tree() = 0;
  static void print_warning(std::string err);
  static void raise_error [[noreturn]] (std::string err);
  static trans_unit *base;
};

#endif /* AST_HPP */