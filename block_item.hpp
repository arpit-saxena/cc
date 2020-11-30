#ifndef BLOCK_ITEM_HPP
#define BLOCK_ITEM_HPP

// TODO: Maybe find a better location for this

#include "ast.hpp"

class blk_item : public ast_node {
 public:
  virtual void dump_tree() override;
};

#endif /* BLOCK_ITEM_HPP */