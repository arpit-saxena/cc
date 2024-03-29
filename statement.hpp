#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include <vector>

#include "ast.hpp"
#include "block_item.hpp"
#include "expression.hpp"

class stmt_node : public blk_item {
 public:
  virtual void dump_tree() override;
  virtual void codegen() override = 0;
  virtual bool has_labeled_stmt() override = 0;
};

class labeled_stmt : public stmt_node {};

// The type other than case or default
// TODO : Think of a better name
class prefix_labeled_stmt : public labeled_stmt {
  std::string identifier;
  stmt_node *statement;

 public:
  prefix_labeled_stmt(const char *ident, stmt_node *stmt);
  void dump_tree() override;
  void codegen() override;
  static llvm::BasicBlock *codegen(std::string name, stmt_node *statement,
                                   bool add_to_table = true);
  bool has_labeled_stmt() override { return true; }
};

class switch_labeled_stmt : public labeled_stmt {
 protected:
  struct scope {
    llvm::SwitchInst *switch_inst;
    llvm::BasicBlock *default_block = nullptr;

    scope(llvm::SwitchInst *s) : switch_inst(s){};
  };

  static std::vector<scope> switch_scopes;
  static scope &top_scope() { return switch_scopes.back(); };
  static void push_scope(llvm::SwitchInst *switch_inst) {
    switch_scopes.emplace_back(switch_inst);
  };
  static void pop_scope() { switch_scopes.pop_back(); };

 public:
  class auto_scope {
   public:
    auto_scope(llvm::SwitchInst *switch_inst) { push_scope(switch_inst); };
    ~auto_scope() { pop_scope(); };
    llvm::BasicBlock *get_default_block() { return top_scope().default_block; }
  };
};

class case_labeled_stmt : public switch_labeled_stmt {
  cond_expr *expression;  // Using as a proxy for constant_expression class
  stmt_node *statement;

 public:
  case_labeled_stmt(cond_expr *expression, stmt_node *statement);
  void dump_tree() override;
  void codegen() override;
  bool has_labeled_stmt() override { return statement->has_labeled_stmt(); }
};

class default_labeled_stmt : public switch_labeled_stmt {
  stmt_node *statement;

 public:
  default_labeled_stmt(stmt_node *statement);
  void dump_tree() override;
  void codegen() override;
  bool has_labeled_stmt() override { return statement->has_labeled_stmt(); }
};

class compound_stmt : public stmt_node {
  std::vector<blk_item *> block_items;

 public:
  compound_stmt *add(blk_item *item);
  void dump_tree() override;
  virtual void codegen() override;
  bool has_labeled_stmt() override {
    for (auto item : block_items) {
      if (item->has_labeled_stmt()) return true;
    }
    return false;
  };
};

class expression_stmt : public stmt_node {
  expr *expression;  // Optional

 public:
  expression_stmt(expr *expression = nullptr);
  void dump_tree() override;
  void codegen() override;
  bool has_labeled_stmt() override { return false; }
};

class selection_stmt : public stmt_node {};

class if_stmt : public selection_stmt {
  expr *condition;
  stmt_node *then_stmt;
  stmt_node *else_stmt;  // Optional

 public:
  if_stmt(expr *cond, stmt_node *then_stmt, stmt_node *else_stmt = nullptr);
  void dump_tree() override;
  void codegen() override;
  bool has_labeled_stmt() override {
    return then_stmt->has_labeled_stmt() ||
           (else_stmt && else_stmt->has_labeled_stmt());
  };
};

class switch_stmt : public selection_stmt {
  expr *expression;
  stmt_node *statement;

 public:
  switch_stmt(expr *expression, stmt_node *statement);
  void dump_tree() override;
  void codegen() override;
  bool has_labeled_stmt() override { return statement->has_labeled_stmt(); };
};

class iteration_stmt : public stmt_node {};

class while_stmt : public iteration_stmt {
  expr *condition;
  stmt_node *statement;

 public:
  while_stmt(expr *condition, stmt_node *statement);
  void dump_tree() override;
  void codegen() override;
  bool has_labeled_stmt() override { return statement->has_labeled_stmt(); };
};

// TODO: Reduce code duplication with while by making a common base
class do_stmt : public iteration_stmt {
  stmt_node *statement;
  expr *condition;

 public:
  do_stmt(stmt_node *statement, expr *condition);
  void dump_tree() override;
  void codegen() override;
  bool has_labeled_stmt() override { return statement->has_labeled_stmt(); }
};

class jump_stmt : public stmt_node {};

class goto_stmt : public jump_stmt {
  std::string identifier;

 public:
  goto_stmt(const char *ident);
  void dump_tree() override;
  void codegen() override;
  bool has_labeled_stmt() override { return false; }
};

class continue_stmt : public jump_stmt {
  static std::vector<llvm::BasicBlock *> dest_blocks;

 public:
  void dump_tree() override;
  void codegen() override;
  bool has_labeled_stmt() override { return false; }

  class scope {
   public:
    scope(llvm::BasicBlock *b) { dest_blocks.push_back(b); }
    ~scope() { dest_blocks.pop_back(); }
  };
};

class break_stmt : public jump_stmt {
  static std::vector<llvm::BasicBlock *> dest_blocks;

 public:
  void dump_tree() override;
  void codegen() override;
  bool has_labeled_stmt() override { return false; }

  class scope {
   public:
    scope(llvm::BasicBlock *b) { dest_blocks.push_back(b); }
    ~scope() { dest_blocks.pop_back(); }
  };
};

class return_stmt : public jump_stmt {
  expr *expression;  // Optional

 public:
  return_stmt(expr *expression = nullptr);
  void dump_tree() override;
  void codegen() override;
  bool has_labeled_stmt() override { return false; }
};

#endif /* STATEMENT_HPP */