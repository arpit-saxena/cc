#include "statement.hpp"

void stmt_node::dump_tree() { cout << "- (statement)" << endl; }

compound_stmt *compound_stmt::add(blk_item *item) {
  block_items.push_back(item);
  return this;
}

void compound_stmt::dump_tree() {
  cout << "- (compound_statement)";
  if (block_items.size() == 0) {
    cout << " EMPTY" << endl;
    return;
  }
  cout << endl;
  cout.indent();
  for (auto block_item : block_items) {
    block_item->dump_tree();
  }
  cout.unindent();
}

void compound_stmt::codegen() {
  for (auto *item : block_items) {
    item->codegen();
  }
}

expression_stmt::expression_stmt(expr *expression) {
  this->expression = expression;
}

void expression_stmt::dump_tree() {
  cout << "- (expression_statement)";
  if (!expression) {
    cout << " EMPTY" << endl;
    return;
  }
  cout << endl;
  cout.indent();
  expression->dump_tree();
  cout.unindent();
}

void expression_stmt::codegen() {
  if (expression) expression->codegen();
}

void jump_stmt::dump_tree() { cout << "- (jump_statement)" << endl; }

return_stmt::return_stmt(expr *expression) { this->expression = expression; }

void return_stmt::dump_tree() {
  cout << "- (return_statement)" << endl;
  if (expression) {
    cout.indent();
    expression->dump_tree();
    cout.unindent();
  }
}