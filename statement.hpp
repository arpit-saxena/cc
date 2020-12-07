#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include <vector>

#include "ast.hpp"
#include "block_item.hpp"
#include "expression.hpp"

class stmt_node : public blk_item {
 public:
  virtual void dump_tree() override;
  virtual void codegen() override {
    raise_error("codegen not implemented for this expression");
  }  // TODO: Make pure virtual
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
};

class compound_stmt : public stmt_node {
  std::vector<blk_item *> block_items;

 public:
  compound_stmt *add(blk_item *item);
  void dump_tree() override;
  virtual void codegen() override;
};

class expression_stmt : public stmt_node {
  expr *expression;  // Optional

 public:
  expression_stmt(expr *expression = nullptr);
  void dump_tree() override;
  void codegen() override;
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
};

class iteration_stmt : public stmt_node {};

class while_stmt : public iteration_stmt {
  expr *condition;
  stmt_node *statement;

 public:
  while_stmt(expr *condition, stmt_node *statement);
  void dump_tree() override;
  void codegen() override;
};

class jump_stmt : public stmt_node {};

class goto_stmt : public jump_stmt {
  std::string identifier;

 public:
  goto_stmt(const char *ident);
  void dump_tree() override;
  void codegen() override;
};

class return_stmt : public jump_stmt {
  expr *expression;  // Optional

 public:
  return_stmt(expr *expression = nullptr);
  void dump_tree() override;
  void codegen() override;
};

#endif /* STATEMENT_HPP */