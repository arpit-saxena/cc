#ifndef BLOCK_ITEM_HPP
#define BLOCK_ITEM_HPP

// TODO: Maybe find a better location for this

#include "ast.hpp"

class blk_item : public ast_node {
 public:
  virtual void dump_tree() override;
  virtual void codegen() = 0;
  virtual bool has_labeled_stmt() = 0;
};

#endif /* BLOCK_ITEM_HPP */