#include "statement.hpp"

blk_item_list *blk_item_list::add(blk_item *item) {
  items.push_back(item);
  return this;
}

void stmt_node::dump_tree() { cout << "- (statement)" << endl; }