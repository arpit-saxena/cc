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

void return_stmt::codegen() {
  llvm::Function *func = sym_table.get_curr_func();
  if (!func) {
    raise_error("Cannot have return statement outside of a function");
  }

  llvm::Type *ret_ty = func->getReturnType();
  if (ret_ty->isVoidTy() && expression) {
    raise_error(
        "Cannot have expression in return statement for function with void "
        "return type");
  }

  if (expression) {
    value ret = expression->codegen();
    expr::convert_to_type(ret, type_i(ret_ty));
    ir_builder.CreateRet(ret.llvm_val);
  } else {
    ir_builder.CreateRetVoid();
  }
}