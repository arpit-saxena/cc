#include "symbol_table.hpp"

llvm::AllocaInst *scope::add_var(llvm::Type *type, std::string name) {
  llvm::AllocaInst *inst = builder.CreateAlloca(type, 0, name.c_str());
  variables[name] = inst;
  return inst;
}

llvm::Function *scope::add_func(llvm::Function *func, std::string name) {
  variables[name] = func;
  return func;
}

llvm::Value *scope::add_val(llvm::Value *val, std::string name) {
  variables[name] = val;
  return val;
}

bool scope::check_var(std::string name) { return variables.count(name) > 0; }

llvm::Value *scope::get_var(std::string name) {
  auto it = variables.find(name);
  if (it == variables.end()) {
    return {};
  }
  return it->second;
}

symbol_table::symbol_table(llvm::IRBuilder<> &global_builder) {
  curr_builder = &global_builder;
  add_scope();
}

void symbol_table::change_func(llvm::Function *func) {
  // TODO: Fix this memory leak
  curr_builder = new llvm::IRBuilder<>(&func->getEntryBlock(),
                                       func->getEntryBlock().begin());
}

void symbol_table::add_scope() { scopes.emplace_back(*curr_builder); }
void symbol_table::pop_scope() { scopes.pop_back(); }
scope *symbol_table::top_scope() { return &scopes.back(); }

bool symbol_table::check_top_scope(std::string name) {
  return scopes.back().check_var(name);
}

llvm::Value *symbol_table::get_var(std::string name) {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    auto ins = it->get_var(name);
    if (ins) return ins;
  }
  return nullptr;
}