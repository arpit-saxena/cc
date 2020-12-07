#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>

#include <string>
#include <unordered_map>
#include <vector>

/*
 * Similar to value, stores llvm::Type along with any other data the frontend
 * needs
 */
struct type_i {  // TODO: Fix name; type_info collides with some header
  llvm::Type *llvm_type;
  bool is_signed;

  type_i(llvm::Type *t = nullptr, bool s = true) : llvm_type(t), is_signed(s) {}
};

/*
 * The frontend needs to maintain more data for a value than a llvm::Value holds
 * such as the signedness, since it determines which instructions to emit.
 * This struct is a wrapper of llvm::Value and any other extra data that is to
 * be maintained
 */
struct value {
  llvm::Value *llvm_val = nullptr;
  bool is_signed = false;

  static value null;
  value() = default;
  value(llvm::Value *val, bool s) : llvm_val(val), is_signed(s) {}
  type_i get_type();
};

class scope {
  std::unordered_map<std::string, value> variables;
  llvm::IRBuilder<> *builder;

 public:
  scope(llvm::IRBuilder<> *builder) : builder(builder){};
  scope(scope &&other) = default;
  llvm::AllocaInst *get_alloca(llvm::Type *llvm_type, std::string name);
  llvm::AllocaInst *add_var(type_i type, std::string name);
  llvm::Function *add_func(llvm::Function *func, std::string name);
  value add_val(value val, std::string name);
  bool check_var(std::string name);
  value get_var(std::string name);
  llvm::IRBuilder<> *get_builder();
};

class func_scope {
  std::vector<scope> scopes;
  llvm::IRBuilder<> *builder;
  bool own_builder;
  llvm::Function *func;

  std::unordered_multimap<std::string, llvm::BranchInst *> pending_gotos;
  std::unordered_map<std::string, llvm::BasicBlock *> label_blocks;

  void push_scope();
  void pop_scope();

 public:
  func_scope(llvm::Function *func);
  func_scope(llvm::IRBuilder<> *builder);
  func_scope(func_scope &&other) = default;
  ~func_scope();
  llvm::Function *get_scope_func();
  // Doesn't add allocation to the symbol table
  llvm::AllocaInst *get_alloca(llvm::Type *llvm_type, std::string name);
  llvm::AllocaInst *add_var(type_i type, std::string name);
  llvm::Function *add_func(llvm::Function *func, std::string name);
  value add_val(value val, std::string name);
  bool check_top_scope(std::string name);

  void add_block_terminator(llvm::Instruction *ins);

  // This class' constructor pushes new scope and its destructor pops the scope
  class auto_scope {
    func_scope *s;

   public:
    auto_scope(func_scope *s) : s(s) { s->push_scope(); }
    ~auto_scope() { s->pop_scope(); }
  };

  auto_scope new_scope() { return auto_scope(this); }

  // Add a label with the given name to the function. Raises an error if the
  // label already exists
  void add_label(std::string name, llvm::BasicBlock *block);

  // Adds a goto instruction through the given builder. If the label is not
  // found yet, the instruction is added to a list of pending instructions and
  // are updated when the label is added
  void add_goto_inst(std::string name, llvm::IRBuilder<> *builder,
                     llvm::BasicBlock *next_block);

  // Searches all scopes from top to bottom for variable defined by name.
  value get_var(std::string name);
};

class symbol_table {
  std::vector<func_scope> func_scopes;
  llvm::IRBuilder<> &global_builder;
  llvm::Function *curr_func;

 public:
  symbol_table(llvm::IRBuilder<> &global_builder);
  void push_func_scope(llvm::Function *func);  // Adds scope to top of the stack
  void pop_func_scope();                       // Pops scope off the stack
  func_scope *top_func_scope();
  llvm::Function *get_curr_func();

  value add_var(type_i type, std::string name);
  llvm::Function *add_func(llvm::Function *func, std::string name);

  // Searches top function scope and global scope for variable defined by name.
  value get_var(std::string name);
};

#endif /* SYMBOL_TABLE_HPP */