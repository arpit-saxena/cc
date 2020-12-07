#include "symbol_table.hpp"

#include "decl_common.hpp"

value value::null;

type_i value::get_type() {
  if (auto alloca = llvm::dyn_cast<llvm::AllocaInst>(llvm_val)) {
    return type_i(alloca->getAllocatedType(), is_signed);
  }
  return type_i(llvm_val->getType(), is_signed);
}

llvm::AllocaInst *scope::get_alloca(llvm::Type *llvm_type, std::string name) {
  llvm::AllocaInst *inst = builder->CreateAlloca(llvm_type, 0, name.c_str());
  return inst;
}

llvm::AllocaInst *scope::add_var(type_i type, std::string name) {
  llvm::Type *llvm_type = type.llvm_type;
  llvm::AllocaInst *inst = get_alloca(llvm_type, name);
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

value scope::add_val(value val, std::string name) {
  variables[name] = val;
  return val;
}

bool scope::check_var(std::string name) { return variables.count(name) > 0; }

value scope::get_var(std::string name) {
  auto it = variables.find(name);
  if (it == variables.end()) {
    return value::null;
  }
  return it->second;
}

llvm::IRBuilder<> *scope::get_builder() { return builder; }

llvm::IRBuilder<> *make_builder(llvm::Function *func) {
  return new llvm::IRBuilder<>(&func->getEntryBlock(),
                               func->getEntryBlock().begin());
}

func_scope::func_scope(llvm::Function *func)
    : labels_scope(make_builder(func)) {
  this->func = func;
  builder = labels_scope.get_builder();
  own_builder = true;
  scopes.emplace_back(builder);
}

func_scope::func_scope(llvm::IRBuilder<> *builder) : labels_scope(builder) {
  this->func = func;
  this->builder = builder;
  own_builder = false;
  scopes.emplace_back(builder);
}

func_scope::~func_scope() {
  if (own_builder) {
    delete builder;
  }
}

llvm::Function *func_scope::get_scope_func() { return func; }

llvm::AllocaInst *func_scope::get_alloca(llvm::Type *llvm_type,
                                         std::string name) {
  return scopes.back().get_alloca(llvm_type, name);
}

llvm::AllocaInst *func_scope::add_var(type_i type, std::string name) {
  return scopes.back().add_var(type, name);
}

llvm::Function *func_scope::add_func(llvm::Function *func, std::string name) {
  return scopes.back().add_func(func, name);
}

value func_scope::add_val(value val, std::string name) {
  return scopes.back().add_val(val, name);
}

bool func_scope::check_top_scope(std::string name) {
  return scopes.back().check_var(name);
}

void func_scope::add_block_terminator(llvm::Instruction *ins) {
  if (!own_builder) {
    std::cout << "Don't own builder, so won't change its insertion point due "
                 "to block terminator\n";
    return;
  }

  builder->SetInsertPoint(ins);
}

void func_scope::push_scope() { scopes.emplace_back(builder); }

void func_scope::pop_scope() { scopes.pop_back(); }

void func_scope::add_label(std::string name, llvm::BasicBlock *block) {
  if (labels_scope.check_var(name)) {
    ast_node::raise_error("Label names must be unique in a function!");
  }

  labels_scope.add_val(value(block, true), name);
}

llvm::BasicBlock *func_scope::get_label(std::string name) {
  value label_val = labels_scope.get_var(name);
  return llvm::dyn_cast_or_null<llvm::BasicBlock>(label_val.llvm_val);
}

value func_scope::get_var(std::string name) {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    auto val = it->get_var(name);
    if (val.llvm_val) return val;
  }
  return value::null;
}

symbol_table::symbol_table(llvm::IRBuilder<> &global_builder)
    : global_builder(global_builder) {
  func_scopes.emplace_back(&global_builder);  // Global scope
}

void symbol_table::push_func_scope(llvm::Function *func) {
  curr_func = func;
  func_scopes.emplace_back(func);
}

void symbol_table::pop_func_scope() {
  func_scopes.pop_back();
  curr_func = func_scopes.back().get_scope_func();
}

func_scope *symbol_table::top_func_scope() { return &func_scopes.back(); }
llvm::Function *symbol_table::get_curr_func() { return curr_func; }

value symbol_table::add_var(type_i type, std::string name) {
  if (func_scopes.size() > 1) {  // Not in global scope currently
    llvm::Value *alloc = func_scopes.back().add_var(type, name);
    return value(alloc, type.is_signed);
  }

  llvm::GlobalVariable *global_var = new llvm::GlobalVariable(
      *ast_node::the_module, type.llvm_type, /* isConstant */ false,
      /* Unsure which linkage to use */ llvm::GlobalValue::PrivateLinkage,
      /* Initializer */ llvm::Constant::getNullValue(type.llvm_type), name);
  value global_val(global_var, type.is_signed);
  return func_scopes.back().add_val(global_val, name);
}

value symbol_table::get_var(std::string name) {
  value val = func_scopes.back().get_var(name);
  if (val.llvm_val) {
    return val;
  }

  if (func_scopes.size() > 1) {  // We have to check global scope too
    return func_scopes.front().get_var(name);
  }

  return value::null;
}