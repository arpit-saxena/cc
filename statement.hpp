#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include <vector>

#include "ast.hpp"
#include "expression.hpp"

// TODO: Maybe move this into separate file when we add declaration too
class blk_item : public ast_node {
 public:
  virtual void dump_tree() override;
};

class stmt_node : public blk_item {
 public:
  virtual void dump_tree() override;
};

class labeled_stmt : public stmt_node {};

class compound_stmt : public stmt_node {
  std::vector<blk_item *> block_items;

 public:
  compound_stmt *add(blk_item *item);
  void dump_tree() override;
};

class expression_stmt : public stmt_node {
  expr *expression;  // Optional

 public:
  expression_stmt(expr *expression = nullptr);
  void dump_tree() override;
};

class selection_stmt : public stmt_node {};
class iteration_stmt : public stmt_node {};
class jump_stmt : public stmt_node {};

#endif /* STATEMENT_HPP */