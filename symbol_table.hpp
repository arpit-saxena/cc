#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>

#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class scope {
 public:
  typedef std::variant<llvm::AllocaInst *, llvm::Function *> var_t;
  typedef std::optional<var_t> var_opt;

 private:
  std::unordered_map<std::string, var_t> variables;
  llvm::IRBuilder<> &builder;

 public:
  scope(llvm::IRBuilder<> &builder) : builder(builder){};
  llvm::AllocaInst *add_var(llvm::Type *type, std::string name);
  llvm::Function *add_func(llvm::Function *func, std::string name);
  bool check_var(std::string name);
  var_opt get_var(std::string name);
};

class symbol_table {
  std::vector<scope> scopes;
  llvm::IRBuilder<> *curr_builder;

 public:
  symbol_table(llvm::IRBuilder<> &global_builder);
  void change_func(llvm::Function *func);
  void add_scope();  // Adds scope to top of the stack
  void pop_scope();  // Pops scope off the stack
  scope *top_scope();

  // Checks if name defines a variable in top scope
  bool check_top_scope(std::string name);

  // Searches all scopes from top to bottom for variable defined by name.
  scope::var_opt get_var(std::string name);
};

#endif /* SYMBOL_TABLE_HPP */