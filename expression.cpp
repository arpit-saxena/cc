#include "expression.hpp"

#include <string>

binary_expr_ops::binary_expr_ops(binary_expr *left, OP op, binary_expr *right) {
  this->left_expr = left;
  this->op = op;
  this->right_expr = right;
}

std::string binary_expr_ops::op_string(binary_expr_ops::OP op) {
  switch (op) {
    case MULT:
      return "*";
    case DIV:
      return "/";
    case MOD:
      return "%";
    case PLUS:
      return "+";
    case MINUS:
      return "-";
    case SHIFT_LEFT:
      return "<<";
    case SHIFT_RIGHT:
      return ">>";
    case LT:
      return "<";
    case GT:
      return ">";
    case LE:
      return "<=";
    case GE:
      return ">=";
    case EQ:
      return "==";
    case NE:
      return "!=";
    case BIT_AND:
      return "&";
    case BIT_XOR:
      return "^";
    case BIT_OR:
      return "|";
    case AND:
      return "&&";
    case OR:
      return "||";
  }

  return "unknown";
}

void binary_expr_ops::dump_tree() {
  cout << "- (binary_expression)" << endl;
  cout.indent();
  left_expr->dump_tree();
  cout << "- (operator) " << op_string(op) << endl;
  right_expr->dump_tree();
  cout.unindent();
}

arg_expr_list *arg_expr_list::add(assign_expr *expr) {
  exprs.push_back(expr);
  return this;
}

void arg_expr_list::dump_tree() {
  cout << "- (argument_expression_list)" << endl;
  cout.indent();
  for (auto &expr : exprs) {
    expr->dump_tree();
  }
  cout.unindent();
}

func_call::func_call(postfix_expr *func_expr, arg_expr_list *expr_list) {
  this->func_expr = func_expr;
  this->arg_list = expr_list;
}

void func_call::dump_tree() {
  cout << "- (functional_call)" << endl;
  cout.indent();
  func_expr->dump_tree();
  if (arg_list) {
    arg_list->dump_tree();
  }
  cout.unindent();
}

ident_expr::ident_expr(const char *id) { identifier = std::string(id); }

void ident_expr::dump_tree() {
  cout << "- (identifier) " << identifier << endl;
}

paren_expr::paren_expr(expr *expression) { this->expression = expression; }

void paren_expr::dump_tree() { expression->dump_tree(); }

int_constant_expr::int_constant_expr(const char *num_str) {
  this->num_str = num_str;
}

void int_constant_expr::dump_tree() {
  cout << "- (constant) " << num_str << endl;
}

string_expr::string_expr(const char *str) : str(str) { free((void *)str); }

void string_expr::dump_tree() { cout << "- (string) " << str << endl; }