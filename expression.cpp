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

void binary_expr_ops::gen_common_type(value lhs, value rhs) {
  auto ltype = llvm::dyn_cast<llvm::IntegerType>(lhs.llvm_val->getType());
  auto rtype = llvm::dyn_cast<llvm::IntegerType>(rhs.llvm_val->getType());

  if (!ltype || !rtype) {
    raise_error("Binary operations on non integer types are not supported!");
  }

  int lbits = ltype->getBitWidth(), rbits = rtype->getBitWidth();

  // This value will be used when number of bits of both operands is same.
  bool is_signed = lhs.is_signed && rhs.is_signed;

  if (lbits != rbits) {
    value val;
    llvm::IntegerType *dest_type;
    if (lbits < rbits) {
      val = lhs;
      dest_type = rtype;
      // FIXME: This is slightly more complicated than this. See 6.3.1.8/1
      // Basically, is unsigned has greater rank, then result is unsigned
      // Else if signed one can represent both signed and unsigned values then
      // result is signed, otherwise the result is unsigned
      is_signed = rhs.is_signed;
    } else {
      val = rhs;
      dest_type = ltype;
      is_signed = lhs.is_signed;
    }

    if (is_signed) {
      val.llvm_val = ir_builder.CreateSExtOrBitCast(val.llvm_val, dest_type,
                                                    "sign extension cast");
    } else {
      val.llvm_val = ir_builder.CreateZExtOrBitCast(val.llvm_val, dest_type,
                                                    "zero extension cast");
    }
  }
}

// Assumes both values have been through proper type conversions
llvm::Instruction::BinaryOps binary_expr_ops::get_arith_op(
    value lhs, value rhs, binary_expr_ops::OP op) {
  bool is_signed = lhs.is_signed && rhs.is_signed;
  typedef llvm::Instruction::BinaryOps llvmOP;
  switch (op) {
    case MULT:
      return llvmOP::Mul;
    case DIV:
      if (is_signed) {
        return llvmOP::SDiv;
      } else {
        return llvmOP::UDiv;
      }
    case MOD:
      if (is_signed) {
        return llvmOP::SRem;
      } else {
        return llvmOP::URem;
      }
    case PLUS:
      return llvmOP::Add;
    case MINUS:
      return llvmOP::Sub;
    case SHIFT_LEFT:
      return llvmOP::Shl;
    case SHIFT_RIGHT:
      if (lhs.is_signed) {
        return llvmOP::AShr;
      } else {
        return llvmOP::LShr;
      }
      break;
    case BIT_XOR:
    case AND:
      lhs.llvm_val = ir_builder.CreateICmpEQ(
          lhs.llvm_val, llvm::ConstantInt::getFalse(the_context));
      rhs.llvm_val = ir_builder.CreateICmpEQ(
          rhs.llvm_val, llvm::ConstantInt::getFalse(the_context));
    case BIT_AND:
      return llvmOP::And;
    case OR:
      lhs.llvm_val = ir_builder.CreateICmpEQ(
          lhs.llvm_val, llvm::ConstantInt::getFalse(the_context));
      rhs.llvm_val = ir_builder.CreateICmpEQ(
          rhs.llvm_val, llvm::ConstantInt::getFalse(the_context));
    case BIT_OR:
      return llvmOP::Or;
  }
  raise_error("No binary operator found!");
}

// Assumes both values have been through proper type conversions
llvm::CmpInst::Predicate binary_expr_ops::get_cmp_pred(value lhs, value rhs,
                                                       OP op) {
  typedef llvm::CmpInst::Predicate pred;

  // FIXME: Assuming both types to be integers
  if (lhs.is_signed && rhs.is_signed) {
    switch (op) {
      case LT:
        return pred::ICMP_SLT;
      case GT:
        return pred::ICMP_SGT;
      case LE:
        return pred::ICMP_SLE;
      case GE:
        return pred::ICMP_SGE;
    }
  } else {
    switch (op) {
      case LT:
        return pred::ICMP_ULT;
      case GT:
        return pred::ICMP_UGT;
      case LE:
        return pred::ICMP_ULE;
      case GE:
        return pred::ICMP_UGE;
    }
  }

  switch (op) {
    case EQ:
      return pred::ICMP_EQ;
    case NE:
      return pred::ICMP_NE;
  }
  raise_error("The operation is not a comparison operator");
}

value binary_expr_ops::codegen() {
  value lhs = left_expr->codegen();
  value rhs = right_expr->codegen();
  gen_common_type(lhs, rhs);
  value ret_val;
  ret_val.is_signed = lhs.is_signed && rhs.is_signed;

  switch (op) {
    case LT:
    case GT:
    case LE:
    case GE:
    case EQ:
    case NE: {
      llvm::CmpInst::Predicate pred = get_cmp_pred(lhs, rhs, op);
      ret_val.llvm_val =
          ir_builder.CreateICmp(pred, lhs.llvm_val, rhs.llvm_val, "comparison");
      break;
    }
    default: {
      llvm::Instruction::BinaryOps llvm_op = get_arith_op(lhs, rhs, op);
      ret_val.llvm_val = ir_builder.CreateBinOp(
          llvm_op, lhs.llvm_val, rhs.llvm_val, "Arithmetic operation");
    }
  }

  return ret_val;
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

const_expr::const_expr(llvm::Constant *data, std::string str) {
  this->data = data;
  this->str = str;
}

// Parse a string representing int, long, long long and their unsigned variants
// into a value and the corresponding type.
// See https://clang.llvm.org/doxygen/LiteralSupport_8cpp_source.html#l00526 for
// clang's implementation
std::pair<llvm::APInt, llvm::Type *> const_expr::get_int(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  assert(str.length() > 0 && "Need non-empty string as number");
  type_specifiers type;

  if (str[0] == 'L' || str[0] == 'U' || str[0] == '\'') {  // => char
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
  return new const_expr(llvm::ConstantInt::get(type, int_val), str);
}

void const_expr::dump_tree() { cout << "- (constant) " << str << endl; }

string_expr::string_expr(const char *str) : str(str) { free((void *)str); }

void string_expr::dump_tree() { cout << "- (string) " << str << endl; }