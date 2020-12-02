#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>

#include <string>
#include <unordered_map>
#include <vector>

class scope {
  std::unordered_map<std::string, llvm::AllocaInst *> variables;
  llvm::IRBuilder<> builder;

 public:
  scope(llvm::Function *func)
      : builder(&func->getEntryBlock(), func->getEntryBlock().begin()){};
  llvm::AllocaInst *add_var(llvm::Type *type, std::string name);
  bool check_var(std::string name);
  llvm::AllocaInst *get_var(std::string name);
};

class symbol_table {
  std::vector<scope> scopes;
  llvm::Function *curr_func;

 public:
  symbol_table(llvm::Function *func);
  void change_func(llvm::Function *func);
  void add_scope();  // Adds scope to top of the stack
  void pop_scope();  // Pops scope off the stack

  // Checks if name defines a variable in top scope
  bool check_top_scope(std::string name);

  // Searches all scopes from top to bottom for variable defined by name.
  // returns nullptr if it doesn't exist
  llvm::AllocaInst *get_var(std::string name);

  // Adds a variable with name on top stack irrespective of whether it already
  // existed on the top scope or not
  llvm::AllocaInst *symbol_table::add_var(llvm::Type *type, std::string name);
};

#endif /* SYMBOL_TABLE_HPP */