#include "statement.hpp"

void stmt_node::dump_tree() { cout << "- (statement)" << endl; }

prefix_labeled_stmt::prefix_labeled_stmt(const char *ident, stmt_node *stmt) {
  identifier = std::string(ident);
  free((void *)ident);
  statement = stmt;
}

void prefix_labeled_stmt::codegen() { codegen(identifier, statement); }

llvm::BasicBlock *prefix_labeled_stmt::codegen(std::string name,
                                               stmt_node *statement,
                                               bool add_to_table) {
  llvm::BasicBlock *block =
      llvm::BasicBlock::Create(the_context, name, sym_table.get_curr_func());

  if (add_to_table) {
    sym_table.top_func_scope()->add_label(name, block);
  }

  llvm::BasicBlock *insert_block = ir_builder.GetInsertBlock();
  if (insert_block->empty()) {
    insert_block->replaceAllUsesWith(block);
    ir_builder.GetInsertBlock()->eraseFromParent();
  } else {
    ir_builder.CreateBr(block);
  }

  ir_builder.SetInsertPoint(block);
  statement->codegen();

  return block;
}

void prefix_labeled_stmt::dump_tree() {
  cout << "- (labeled_statement)" << endl;
  cout.indent();
  cout << "- (label) " << identifier << endl;
  statement->dump_tree();
  cout.unindent();
}

std::vector<switch_labeled_stmt::scope> switch_labeled_stmt::switch_scopes;

case_labeled_stmt::case_labeled_stmt(cond_expr *expression,
                                     stmt_node *statement) {
  this->expression = expression;
  this->statement = statement;
}

void case_labeled_stmt::dump_tree() {
  cout << "- (labeled_statement) case" << endl;
  cout.indent();
  expression->dump_tree();
  statement->dump_tree();
  cout.unindent();
}

void case_labeled_stmt::codegen() {
  value val = expression->codegen();
  auto const_val = llvm::dyn_cast<llvm::ConstantInt>(val.llvm_val);

  if (!const_val) {
    raise_error("case statement must contain an integer constant expression");
  }

  std::string block_name = "case_";
  block_name += const_val->getValue().toString(/* radix */ 10, val.is_signed);

  auto block = prefix_labeled_stmt::codegen(block_name, statement,
                                            /*add_to_table */ false);

  top_scope().switch_inst->addCase(const_val, block);
}

default_labeled_stmt::default_labeled_stmt(stmt_node *statement) {
  this->statement = statement;
}

void default_labeled_stmt::dump_tree() {
  cout << "- (default)" << endl;
  cout.indent();
  statement->dump_tree();
  cout.unindent();
}

void default_labeled_stmt::codegen() {
  if (top_scope().default_block) {
    // Some default statement is already present
    raise_error("Multiple default cases are not allowed in a switch statement");
  }

  top_scope().default_block = prefix_labeled_stmt::codegen(
      "default", statement, /* add_to_table */ false);
}

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
  auto s = sym_table.top_func_scope()->new_scope();
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

if_stmt::if_stmt(expr *condition, stmt_node *then_stmt, stmt_node *else_stmt)
    : condition(condition), then_stmt(then_stmt), else_stmt(else_stmt) {}

void if_stmt::dump_tree() {
  cout << "- (if_statement)" << endl;
  cout.indent();
  condition->dump_tree();
  then_stmt->dump_tree();
  if (else_stmt) else_stmt->dump_tree();
  cout.unindent();
}

// condition, then_stmt, else_stmt
void if_stmt::codegen() {
  auto s = sym_table.top_func_scope()->new_scope();

  value cond_val = condition->codegen();

  if (auto const_cond = llvm::dyn_cast<llvm::Constant>(cond_val.llvm_val)) {
    if (const_cond->isZeroValue() && else_stmt) {
      auto s = sym_table.top_func_scope()->new_scope();
      else_stmt->codegen();
    } else {
      auto s = sym_table.top_func_scope()->new_scope();
      then_stmt->codegen();
    }
    return;
  }

  llvm::Value *binary_cond = ir_builder.CreateICmpNE(
      cond_val.llvm_val, const_expr::get_val(0, cond_val.get_type()).llvm_val);

  llvm::BasicBlock *then_block =
      llvm::BasicBlock::Create(the_context, "then", sym_table.get_curr_func());
  llvm::BasicBlock *else_block;
  llvm::BasicBlock *end_block = llvm::BasicBlock::Create(
      the_context, "if_end", sym_table.get_curr_func());

  if (else_stmt) {
    else_block = llvm::BasicBlock::Create(the_context, "else",
                                          sym_table.get_curr_func());
  } else {
    else_block = end_block;
  }

  auto inst = ir_builder.CreateCondBr(binary_cond, then_block, else_block);

  {  // New scope
    auto s = sym_table.top_func_scope()->new_scope();

    ir_builder.SetInsertPoint(then_block);
    then_stmt->codegen();
    if (!ir_builder.GetInsertBlock()->getTerminator()) {
      ir_builder.CreateBr(end_block);
    }
  }

  if (else_stmt) {
    auto s = sym_table.top_func_scope()->new_scope();

    ir_builder.SetInsertPoint(else_block);
    else_stmt->codegen();
    if (!ir_builder.GetInsertBlock()->getTerminator()) {
      ir_builder.CreateBr(end_block);
    }
  }

  ir_builder.SetInsertPoint(end_block);
}

switch_stmt::switch_stmt(expr *expression, stmt_node *statement) {
  this->expression = expression;
  this->statement = statement;
}

void switch_stmt::dump_tree() {
  cout << "- (switch_statement)" << endl;
  cout.indent();
  expression->dump_tree();
  statement->dump_tree();
  cout.unindent();
}

void switch_stmt::codegen() {
  auto sym_scope = sym_table.top_func_scope()->new_scope();

  value control_var = expression->codegen();

  auto end_block = llvm::BasicBlock::Create(the_context, "switch_end");
  auto switch_inst = ir_builder.CreateSwitch(control_var.llvm_val, end_block);

  // This is mainly to satisfy the invariant of always having a non terminated
  // block being used the IRBuilder
  auto new_block = llvm::BasicBlock::Create(the_context, "switch_new",
                                            sym_table.get_curr_func());
  ir_builder.SetInsertPoint(new_block);

  auto switch_scope = switch_labeled_stmt::auto_scope(switch_inst);

  {  // New symbol scope
    auto sym_scope = sym_table.top_func_scope()->new_scope();
    statement->codegen();
  }

  if (auto default_block = switch_scope.get_default_block()) {
    switch_inst->setDefaultDest(default_block);
  } else {
    ir_builder.CreateBr(end_block);
    end_block->insertInto(sym_table.get_curr_func());
    ir_builder.SetInsertPoint(end_block);
  }
}

while_stmt::while_stmt(expr *condition, stmt_node *statement)
    : condition(condition), statement(statement) {}

void while_stmt::dump_tree() {
  cout << "- (while_statement)" << endl;
  cout.indent();
  condition->dump_tree();
  statement->dump_tree();
  cout.unindent();
}

void while_stmt::codegen() {
  auto s = sym_table.top_func_scope()->new_scope();

  llvm::BasicBlock *cond_block = llvm::BasicBlock::Create(
      the_context, "while_cond", sym_table.get_curr_func());
  llvm::BasicBlock *body_block = llvm::BasicBlock::Create(
      the_context, "while_body", sym_table.get_curr_func());
  llvm::BasicBlock *end_block = llvm::BasicBlock::Create(
      the_context, "while_end", sym_table.get_curr_func());

  ir_builder.CreateBr(cond_block);

  ir_builder.SetInsertPoint(cond_block);
  value cond_val = condition->codegen();
  llvm::Value *binary_cond = ir_builder.CreateICmpNE(
      cond_val.llvm_val, const_expr::get_val(0, cond_val.get_type()).llvm_val);
  ir_builder.CreateCondBr(binary_cond, body_block, end_block);

  {  // New scope
    auto s = sym_table.top_func_scope()->new_scope();

    ir_builder.SetInsertPoint(body_block);
    statement->codegen();
    ir_builder.CreateBr(cond_block);
  }

  ir_builder.SetInsertPoint(end_block);
}

goto_stmt::goto_stmt(const char *ident) {
  identifier = std::string(ident);
  free((void *)ident);
}

void goto_stmt::dump_tree() {
  cout << "- (jump_statement) goto" << endl;
  cout.indent();
  cout << "- " << identifier << endl;
  cout.unindent();
}

void goto_stmt::codegen() {
  auto next_block = llvm::BasicBlock::Create(the_context, "goto_end",
                                             sym_table.get_curr_func());
  sym_table.top_func_scope()->add_goto_inst(identifier, &ir_builder,
                                            next_block);
}

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
  } else if (!ret_ty->isVoidTy() && !expression) {
    raise_error(
        "Cannot have return statement without an expression in a function with "
        "non-void return type");
  }

  /* if (ir_builder.GetInsertBlock()->getTerminator()) {
    if (expression) {
      print_warning("Unreachable code");
    }
    return;
  } */

  if (expression) {
    value ret = expression->codegen();
    expr::convert_to_type(ret, type_i(ret_ty));
    ir_builder.CreateRet(ret.llvm_val);
  } else {
    ir_builder.CreateRetVoid();
  }

  auto next_block = llvm::BasicBlock::Create(the_context, "return_end",
                                             sym_table.get_curr_func());
  ir_builder.SetInsertPoint(next_block);
}