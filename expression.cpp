#include "expression.hpp"

#include <algorithm>
#include <cstring>
#include <string>

#include "decl_common.hpp"

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

const_expr::const_expr(llvm::ConstantData *data) { this->data = data; }

// Parse a string representing int, long, long long and their unsigned variants
// into a value and the corresponding type.
// See https://clang.llvm.org/doxygen/LiteralSupport_8cpp_source.html#l00526 for
// clang's implementation
std::pair<llvm::APInt, llvm::Type *> const_expr::get_int(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  assert(str.length() > 0 && "Need non-empty string as number");
  type_specifiers type;

  if (str[0] == 'L' || str[0] == 'U' || str[0] = '\'') {  // => char
    if (str[0] == 'L' || str[0] == 'U') {
      raise_error("No support for wchar_t or char16_t or char32_t");
    }

    type.add_spec(type_specifiers::CHAR);
    char c = str[1];  // TODO: Add support for escape sequences etc
    return {llvm::APInt(8, c), type.get_type()};
  }

  int start = 0, end = str.length() - 1;
  int radix = 10;
  if (str[0] == '0') {                        // => Octal or Hexa
    if (str.length() > 1 && str[1] == 'X') {  // => Hexa
      radix = 16;
      start = 2;
    } else {
      radix = 8;
      if (str.length() > 1) start = 1;  // Since only 0 might be present
    }
  }

  bool is_unsigned = false, is_long = false, is_longlong = false;
  auto it = str.rbegin();
  if (*it == 'U') {
    is_unsigned = true;
    end--;
    ++it;
  }

  if (it != str.rend() && *it == 'L') {
    ++it;
    end--;
    if (it != str.rend() && *it == 'L') {
      is_longlong = true;
      ++it;
      end--;
    } else {
      is_long = true;
    }
  }

  if (it != str.rend() && *it == 'U') {
    is_unsigned = true;
    end--;
  }

  int num_bits;
  if (is_long) {
    num_bits = 32;
    type.add_spec(type_specifiers::LONG);
  } else if (is_longlong) {
    num_bits = 64;
    type.add_spec(type_specifiers::LONG_LONG);
  } else {
    num_bits = 32;
    type.add_spec(type_specifiers::INT);
  }

  if (is_unsigned) {
    type.add_spec(type_specifiers::UNSIGNED);
  }

  // Not exactly as the spec wants since in the spec a literal with no suffix
  // can also have a type long long if it can't be represented by the int type
  // See 6.4.4.1/5
  return {
      llvm::APInt(num_bits, str.substr(start, end - start + 1), !is_unsigned),
      type.get_type()};
}

const_expr *const_expr::new_int_expr(const char *s) {
  std::string str(s);
  free((void *)s);
  auto [int_val, type] = get_int(str);
  return new const_expr(llvm::ConstantInt::get(type, int_val));
}

int_constant_expr::int_constant_expr(const char *num_str) {
  this->num_str = num_str;
}

void int_constant_expr::dump_tree() {
  cout << "- (constant) " << num_str << endl;
}

string_expr::string_expr(const char *str) : str(str) { free((void *)str); }

void string_expr::dump_tree() { cout << "- (string) " << str << endl; }