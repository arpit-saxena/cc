#include "symbol_table.hpp"

#include "decl_common.hpp"

value value::null;

type_i value::get_type() { return type_i(llvm_val->getType(), is_signed); }

/* type_i::type_i(type_i &&type) {
  llvm_type = type.llvm_type;
  is_signed = type.is_signed;
} */

llvm::AllocaInst *scope::add_var(type_i type, std::string name) {
  llvm::Type *llvm_type = type.llvm_type;
  llvm::AllocaInst *inst = builder.CreateAlloca(llvm_type, 0, name.c_str());
  value val(inst, type.is_signed);
  val.llvm_val = inst;
  variables[name] = val;
  return inst;
}

llvm::Function *scope::add_func(llvm::Function *func, std::string name) {
  value val(func, true);
  variables[name] = val;
  return func;
}

bool scope::check_var(std::string name) { return variables.count(name) > 0; }

value scope::get_var(std::string name) {
  auto it = variables.find(name);
  if (it == variables.end()) {
    return value::null;
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

value symbol_table::get_var(std::string name) {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    auto val = it->get_var(name);
    if (val.llvm_val) return val;
  }
  return value::null;
}