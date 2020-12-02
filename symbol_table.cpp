#include "symbol_table.hpp"

llvm::AllocaInst *scope::add_var(llvm::Type *type, std::string name) {
  llvm::AllocaInst *inst = builder.CreateAlloca(type, 0, name.c_str());
  variables[name] = inst;
  return inst;
}

bool scope::check_var(std::string name) { return variables.count(name) > 0; }

llvm::AllocaInst *scope::get_var(std::string name) {
  auto it = variables.find(name);
  if (it == variables.end()) {
    return nullptr;
  }
  return it->second;
}

symbol_table::symbol_table(llvm::Function *func) { curr_func = func; }

void symbol_table::change_func(llvm::Function *func) { curr_func = func; }

void symbol_table::add_scope() { scopes.emplace_back(curr_func); }

void symbol_table::pop_scope() { scopes.pop_back(); }

bool symbol_table::check_top_scope(std::string name) {
  return scopes.back().check_var(name);
}

llvm::AllocaInst *symbol_table::get_var(std::string name) {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    llvm::AllocaInst *ins = it->get_var(name);
    if (ins) return ins;
  }
  return nullptr;
}

llvm::AllocaInst *symbol_table::add_var(llvm::Type *type, std::string name) {
  return scopes.back().add_var(type, name);
}