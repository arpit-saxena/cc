#include "expression.hpp"

#include <algorithm>
#include <cstring>
#include <string>

#include "decl_common.hpp"

void expr::convert_to_bool(llvm::Value *&val) {
  val = ir_builder.CreateICmpEQ(val, const_expr::get_val(0).llvm_val);
  val = ir_builder.CreateZExtOrBitCast(val,
                                       llvm::IntegerType::get(the_context, 32));
}

cond_expr_ops::cond_expr_ops(binary_expr *cond, expr *true_expr,
                             cond_expr *false_expr) {
  this->cond = cond;
  this->true_expr = true_expr;
  this->false_expr = false_expr;
}

value cond_expr_ops::codegen() {
  value cond_val = cond->codegen();
  cond_val.llvm_val = ir_builder.CreateTruncOrBitCast(
      cond_val.llvm_val, llvm::IntegerType::get(the_context, 1));
  llvm::BasicBlock *curr_block = ir_builder.GetInsertBlock();
  llvm::AllocaInst *result = sym_table.top_scope()->get_alloca(
      llvm::IntegerType::get(the_context, 32), "res");

  llvm::BasicBlock *true_block =
      llvm::BasicBlock::Create(the_context, "true", sym_table.get_curr_func());
  ir_builder.SetInsertPoint(true_block);
  value true_val = true_expr->codegen();
  ir_builder.CreateStore(true_val.llvm_val, result);

  llvm::BasicBlock *false_block =
      llvm::BasicBlock::Create(the_context, "false", sym_table.get_curr_func());
  ir_builder.SetInsertPoint(false_block);
  value false_val = false_expr->codegen();
  ir_builder.CreateStore(false_val.llvm_val, result);

  ir_builder.SetInsertPoint(curr_block);
  ir_builder.CreateCondBr(cond_val.llvm_val, true_block, false_block);

  llvm::BasicBlock *next_block =
      llvm::BasicBlock::Create(the_context, "", sym_table.get_curr_func());
  ir_builder.SetInsertPoint(true_block);
  ir_builder.CreateBr(next_block);
  ir_builder.SetInsertPoint(false_block);
  ir_builder.CreateBr(next_block);

  ir_builder.SetInsertPoint(next_block);
  value ret;
  ret.llvm_val = ir_builder.CreateLoad(result, "res");

  return ret;
}

void cond_expr_ops::dump_tree() {
  cout << "- (conditional_expression)" << endl;
  cout.indent();
  cond->dump_tree();
  true_expr->dump_tree();
  false_expr->dump_tree();
  cout.unindent();
}

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
      val.llvm_val = ir_builder.CreateSExtOrBitCast(val.llvm_val, dest_type);
    } else {
      val.llvm_val = ir_builder.CreateZExtOrBitCast(val.llvm_val, dest_type);
    }
  }
}

// Assumes both values have been through proper type conversions
llvm::Instruction::BinaryOps binary_expr_ops::get_arith_op(
    value &lhs, value &rhs, binary_expr_ops::OP op) {
  bool is_signed = lhs.is_signed && rhs.is_signed;
  llvm::Type *common_type = lhs.llvm_val->getType();
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
      return llvmOP::Xor;
    case AND:
      convert_to_bool(lhs.llvm_val);
      convert_to_bool(rhs.llvm_val);
    case BIT_AND:
      return llvmOP::And;
    case OR:
      convert_to_bool(lhs.llvm_val);
      convert_to_bool(rhs.llvm_val);
    case BIT_OR:
      return llvmOP::Or;
  }
  raise_error("No binary operator found!");
}

// Assumes both values have been through proper type conversions
llvm::CmpInst::Predicate binary_expr_ops::get_cmp_pred(value &lhs, value &rhs,
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
  return codegen(lhs, op, rhs);
}

value binary_expr_ops::codegen(value lhs, OP op, value rhs) {
  gen_common_type(lhs, rhs);
  value ret_val;
  ret_val.is_signed = lhs.is_signed && rhs.is_signed;
  llvm::Type *common_type = lhs.llvm_val->getType();

  switch (op) {
    case LT:
    case GT:
    case LE:
    case GE:
    case EQ:
    case NE: {
      llvm::CmpInst::Predicate pred = get_cmp_pred(lhs, rhs, op);
      ret_val.llvm_val =
          ir_builder.CreateICmp(pred, lhs.llvm_val, rhs.llvm_val);
      ret_val.llvm_val =
          ir_builder.CreateZExtOrBitCast(ret_val.llvm_val, common_type);
      break;
    }
    default: {
      llvm::Instruction::BinaryOps llvm_op = get_arith_op(lhs, rhs, op);
      ret_val.llvm_val =
          ir_builder.CreateBinOp(llvm_op, lhs.llvm_val, rhs.llvm_val);
    }
  }

  return ret_val;
}

unary_op_expr::unary_op_expr(unary_op_expr::OP op, cast_expr *expression) {
  this->op = op;
  this->expression = expression;
}

std::string unary_op_expr::op_string(unary_op_expr::OP op) {
  switch (op) {
    case ADDRESS_OF:
      return "&";
    case INDIRECTION:
      return "*";
    case PLUS:
      return "+";
    case MINUS:
      return "-";
    case BIT_NOT:
      return "~";
    case NOT:
      return "!";
  }
  raise_error("Unkown unary operator");
}

void unary_op_expr::dump_tree() {
  cout << "- (unary_expression)" << endl;
  cout.indent();
  cout << "- (operator) " << op_string(op) << endl;
  expression->dump_tree();
  cout.unindent();
}

value unary_op_expr::codegen() {
  value val = expression->codegen();
  return codegen(op, val);
}

value unary_op_expr::codegen(OP op, value val) {
  typedef binary_expr_ops::OP bin_op;
  switch (op) {
    case ADDRESS_OF:
    case INDIRECTION:
      raise_error("& and * operators are not supported!");
    case PLUS:
      return val;  // TODO: Change this when adding support for floating points
    case MINUS:
      return binary_expr_ops::codegen(const_expr::get_val(0), bin_op::MINUS,
                                      val);
    case BIT_NOT:
      return binary_expr_ops::codegen(const_expr::get_val(-1), bin_op::BIT_XOR,
                                      val);
    case NOT:
      return binary_expr_ops::codegen(const_expr::get_val(0), bin_op::EQ, val);
  }
  raise_error("Unkown unary operator!");
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

value ident_expr::codegen() {
  value val = sym_table.get_var(identifier);
  if (!val.llvm_val) {
    raise_error("use of identifier before declaration");
  }
  val.llvm_val = ir_builder.CreateLoad(val.llvm_val, identifier);
  return val;
}

paren_expr::paren_expr(expr *expression) { this->expression = expression; }

void paren_expr::dump_tree() { expression->dump_tree(); }

value paren_expr::codegen() { return expression->codegen(); }

const_expr::const_expr(llvm::Constant *data, bool is_signed, std::string str) {
  this->data = data;
  this->is_signed = is_signed;
  this->str = str;
}

// Parse a string representing int, long, long long and their unsigned
// variants into a value and the corresponding type. See
// https://clang.llvm.org/doxygen/LiteralSupport_8cpp_source.html#l00526 for
// clang's implementation
const_expr *const_expr::new_int_expr(const char *s) {
  std::string str(s);
  free((void *)s);
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
    llvm::APInt int_val = llvm::APInt(8, c);
    return new const_expr(
        llvm::ConstantInt::get(type.get_type().llvm_type, int_val), false, str);
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
  llvm::APInt int_val =
      llvm::APInt(num_bits, str.substr(start, end - start + 1), radix);
  /* llvm::outs() << "Got val ";
  int_val.print(llvm::outs(), !is_unsigned);
  llvm::outs() << " from string " << str << '\n'; */
  return new const_expr(
      llvm::ConstantInt::get(type.get_type().llvm_type, int_val), !is_unsigned,
      str);
}

value const_expr::get_val(int num) {
  value val;
  val.is_signed = true;
  llvm::Type *type = type_specifiers(type_specifiers::INT).get_type().llvm_type;
  val.llvm_val = llvm::ConstantInt::get(type, num, true);
  return val;
}

void const_expr::dump_tree() { cout << "- (constant) " << str << endl; }

value const_expr::codegen() {
  value ret_val;
  ret_val.llvm_val = data;
  ret_val.is_signed = is_signed;
  return ret_val;
}

string_expr::string_expr(const char *str) : str(str) { free((void *)str); }

void string_expr::dump_tree() { cout << "- (string) " << str << endl; }