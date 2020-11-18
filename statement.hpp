#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include <vector>

#include "ast.hpp"

// TODO: Maybe move this into separate file when we add declaration too
class blk_item : public ast_node {};

class blk_item_list : public ast_node {
  std::vector<blk_item *> items;

 public:
  blk_item_list *add(blk_item *item);
};

class stmt_node : public blk_item {
 public:
  void dump_tree() override;
};

class labeled_stmt : public stmt_node {};
class compound_stmt : public stmt_node {};
class expression_stmt : public stmt_node {};
class selection_stmt : public stmt_node {};
class iteration_stmt : public stmt_node {};
class jump_stmt : public stmt_node {};

#endif /* STATEMENT_HPP */