#include "expression.hpp"

#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>

#include "decl_common.hpp"

void expr::convert_to_bool(llvm::Value *&val) {
  val = ir_builder.CreateICmpEQ(
      val, const_expr::get_val(0, val->getType()).llvm_val);
  val = ir_builder.CreateZExtOrBitCast(val,
                                       llvm::IntegerType::get(the_context, 32));
}

void expr::convert_to_type(value &val, type_i rtype) {
  auto ltype_llvm = llvm::dyn_cast<llvm::IntegerType>(val.llvm_val->getType());
  auto rtype_llvm = llvm::dyn_cast<llvm::IntegerType>(rtype.llvm_type);

  if (!ltype_llvm || !rtype_llvm) {
    raise_error("Cannot convert non integer type!");
  }

  int lbits = ltype_llvm->getBitWidth(), rbits = rtype_llvm->getBitWidth();

  if (val.is_signed) {
    val.llvm_val = ir_builder.CreateSExtOrTrunc(val.llvm_val, rtype_llvm);
  } else {
    val.llvm_val = ir_builder.CreateZExtOrTrunc(val.llvm_val, rtype_llvm);
  }

  val.is_signed = rtype.is_signed;
}

void expr::gen_common_type(value &lhs, value &rhs) {
  type_i dest_type = get_common_type(lhs.get_type(), rhs.get_type());

  // Only one value will be converted but since get_common_type doesn't give any
  // info on which value to convert, we have to do this. Relies on the fact that
  // CreateSExtOrTrunc will return the value as is if it is already the
  // destination type. Could probably use CreateSExt too but the docs don't
  // specify that it doesn't generate an op if value already has desired type

  if (lhs.is_signed) {
    lhs.llvm_val =
        ir_builder.CreateSExtOrTrunc(lhs.llvm_val, dest_type.llvm_type);
  } else {
    lhs.llvm_val =
        ir_builder.CreateZExtOrTrunc(lhs.llvm_val, dest_type.llvm_type);
  }

  if (rhs.is_signed) {
    rhs.llvm_val =
        ir_builder.CreateSExtOrTrunc(rhs.llvm_val, dest_type.llvm_type);
  } else {
    rhs.llvm_val =
        ir_builder.CreateZExtOrTrunc(rhs.llvm_val, dest_type.llvm_type);
  }
}

type_i expr::get_common_type(type_i t1, type_i t2) {
  auto ltype = llvm::dyn_cast<llvm::IntegerType>(t1.llvm_type);
  auto rtype = llvm::dyn_cast<llvm::IntegerType>(t2.llvm_type);

  if (!ltype || !rtype) {
    raise_error("Common types for non integer types not defined!");
  }

  int lbits = ltype->getBitWidth(), rbits = rtype->getBitWidth();

  type_i ret = t1;
  if (lbits == rbits) {
    ret.is_signed = t1.is_signed && t2.is_signed;
    return ret;
  }

  value val;
  if (lbits < rbits) {
    ret.llvm_type = rtype;
    // FIXME: This is slightly more complicated than this. See 6.3.1.8/1
    // Basically, if unsigned has greater rank, then result is unsigned
    // Else if signed one can represent both signed and unsigned values then
    // result is signed, otherwise the result is unsigned
    ret.is_signed = t2.is_signed;
  } else {
    ret.llvm_type = ltype;
    ret.is_signed = t1.is_signed;
  }

  return ret;
}

type_i expr::get_common_type(expr *e1, expr *e2) {
  return get_common_type(e1->get_type(), e2->get_type());
}

cond_expr_ops::cond_expr_ops(binary_expr *cond, expr *true_expr,
                             cond_expr *false_expr) {
  this->cond = cond;
  this->true_expr = true_expr;
  this->false_expr = false_expr;
}

value cond_expr_ops::codegen() {
  value cond_val = cond->codegen();
  sym_table.top_func_scope()->push_scope();
  value binary_cond = sym_table.add_var(ir_builder.getInt1Ty(), "cond");
  binary_cond.llvm_val = ir_builder.CreateICmpNE(
      cond_val.llvm_val, const_expr::get_val(0, cond_val.get_type()).llvm_val);
  sym_table.top_func_scope()->pop_scope();
  return codegen(binary_cond, true_expr, false_expr);
}

value cond_expr_ops::codegen(value cond_val, expr *true_expr,
                             expr *false_expr) {
  type_i res_type = get_common_type(true_expr, false_expr);

  sym_table.top_func_scope()->push_scope();
  value result = sym_table.add_var(res_type, "res");

  llvm::BasicBlock *curr_block = ir_builder.GetInsertBlock();

  llvm::BasicBlock *true_block =
      llvm::BasicBlock::Create(the_context, "true", sym_table.get_curr_func());
  llvm::BasicBlock *false_block =
      llvm::BasicBlock::Create(the_context, "false", sym_table.get_curr_func());
  ir_builder.CreateCondBr(cond_val.llvm_val, true_block, false_block);
  llvm::BasicBlock *next_block = llvm::BasicBlock::Create(
      the_context, "cond_end", sym_table.get_curr_func());

  ir_builder.SetInsertPoint(next_block);
  value ret = create_load(result, "res");

  sym_table.top_func_scope()->pop_scope();

  ir_builder.SetInsertPoint(true_block);
  value true_val = true_expr->codegen();
  convert_to_type(true_val, res_type);
  ir_builder.CreateStore(true_val.llvm_val, result.llvm_val);
  ir_builder.CreateBr(next_block);

  ir_builder.SetInsertPoint(false_block);
  value false_val = false_expr->codegen();
  convert_to_type(false_val, res_type);
  ir_builder.CreateStore(false_val.llvm_val, result.llvm_val);
  ir_builder.CreateBr(next_block);

  ir_builder.SetInsertPoint(next_block);

  // curr_block->print(llvm::outs());

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

type_i cond_expr_ops::get_type() {
  return get_common_type(true_expr->get_type(), false_expr->get_type());
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
    case BIT_AND:
      return llvmOP::And;
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

value binary_expr_ops::codegen() { return codegen(left_expr, op, right_expr); }

value binary_expr_ops::codegen(expr *lhs_expr, OP op, expr *rhs_expr) {
  value lhs = lhs_expr->codegen();

  value zero_val = const_expr::get_val(0, lhs.get_type().llvm_type);
  value one_val = const_expr::get_val(1, lhs.get_type().llvm_type);

  switch (op) {
    case AND: {
      lhs.llvm_val = ir_builder.CreateICmpNE(lhs.llvm_val, zero_val.llvm_val);
      value_expr false_expr(zero_val);
      value val = cond_expr_ops::codegen(lhs, rhs_expr, &false_expr);
      return binary_expr_ops::codegen(val, binary_expr_ops::NE, zero_val);
    }
    case OR: {
      lhs.llvm_val = ir_builder.CreateICmpNE(lhs.llvm_val, zero_val.llvm_val);
      value_expr true_expr(one_val);
      value val = cond_expr_ops::codegen(lhs, &true_expr, rhs_expr);
      return binary_expr_ops::codegen(val, binary_expr_ops::NE, zero_val);
    }
  }

  value rhs = rhs_expr->codegen();
  return codegen(lhs, op, rhs);
}

value binary_expr_ops::codegen(value lhs, OP op, value rhs) {
  gen_common_type(lhs, rhs);
  value ret_val;
  ret_val.is_signed = lhs.is_signed && rhs.is_signed;
  llvm::Type *common_type = lhs.llvm_val->getType();

  if (op == AND || op == OR) {
    convert_to_bool(lhs.llvm_val);
    convert_to_bool(rhs.llvm_val);

    if (op == AND) {
      op = BIT_AND;
    } else {
      op = BIT_OR;
    }
  }

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

type_i binary_expr_ops::get_type() {
  return get_common_type(left_expr->get_type(), right_expr->get_type());
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
      return binary_expr_ops::codegen(const_expr::get_val(0, val.get_type()),
                                      bin_op::MINUS, val);
    case BIT_NOT:
      return binary_expr_ops::codegen(const_expr::get_val(-1, val.get_type()),
                                      bin_op::BIT_XOR, val);
    case NOT:
      return binary_expr_ops::codegen(const_expr::get_val(0, val.get_type()),
                                      bin_op::EQ, val);
  }
  raise_error("Unkown unary operator!");
}

type_i unary_op_expr::get_type() {
  switch (op) {
    case PLUS:
    case MINUS:
    case BIT_NOT:
    case NOT:
      // TODO: Change this when adding support for other types
      return expression->get_type();
  }
  raise_error("Address-of and indirection operators are not supported!");
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

std::vector<type_i> arg_expr_list::get_types() {
  std::vector<type_i> types;
  types.reserve(exprs.size());
  std::transform(exprs.begin(), exprs.end(), std::back_inserter(types),
                 [](expr *e) { return e->get_type(); });
  return types;
}

std::vector<value> arg_expr_list::codegen() {
  std::vector<value> vals;
  vals.reserve(exprs.size());
  std::transform(exprs.begin(), exprs.end(), std::back_inserter(vals),
                 [](expr *e) { return e->codegen(); });
  return vals;
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

value func_call::codegen() {
  value func = func_expr->codegen();
  auto *llvm_func = llvm::dyn_cast<llvm::Function>(func.llvm_val);
  if (!llvm_func) {
    raise_error("Function value not generated by expression in func_call");
  }

  std::vector<value> arg_vals;
  if (arg_list) {
    arg_vals = arg_list->codegen();
  }

  int i = 0;
  for (auto &llvm_arg : llvm_func->args()) {
    if (i >= arg_vals.size()) {
      // This is necessary because the spec says "empty list in a function
      // declarator that is not part of a definition of that function
      // specifies that no information about the number or types of
      // the parameters is supplied.", so number of arguments can differ from
      // number of parameters in declaration and still be valid
      break;
    }

    if (auto int_type = llvm::dyn_cast<llvm::IntegerType>(llvm_arg.getType())) {
      // We only want to attempt to convert integer types for now
      convert_to_type(arg_vals[i++], type_i(int_type));
    }
  }

  std::vector<llvm::Value *> llvm_arg_vals;
  std::transform(arg_vals.begin(), arg_vals.end(),
                 std::back_inserter(llvm_arg_vals),
                 [](value val) { return val.llvm_val; });

  // TODO: Add type checking for args. Will involve somehow modifying the value
  // struct to hold types for the function's return value and parameters.
  // Perhaps create a subclass for value. Would require many changes throughout
  // since value is being used by-value everywhere

  i = 0;
  for (auto &arg : llvm_func->args()) {
    if (arg.getType() != llvm_arg_vals[i++]->getType()) {
      raise_error(
          "Given argument does not match type of function parameter. Note that "
          "currently arguments given to a function call do not support some"
          "implicit type conversions");
    }
  }

  llvm::Value *ret = ir_builder.CreateCall(llvm_func, llvm_arg_vals);

  // FIXME: For now, symbol table always makes is_signed true for any functions
  // added to it.
  return value(ret, func.is_signed);  // Assuming func contains signedness info
                                      // about the return type
}

type_i func_call::get_type() {
  type_i func_type = func_expr->get_type();
  llvm::FunctionType *ll_func_type;
  if (auto ll_func_ptr =
          llvm::dyn_cast<llvm::PointerType>(func_type.llvm_type)) {
    ll_func_type =
        llvm::dyn_cast<llvm::FunctionType>(ll_func_ptr->getElementType());
  }

  if (!ll_func_type) {
    raise_error("Type of expression in function call is not function!");
  }

  return type_i(ll_func_type->getReturnType(), func_type.is_signed);
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

  return create_load(val, identifier);
}

type_i ident_expr::get_type() {
  value val = sym_table.get_var(identifier);
  if (!val.llvm_val) {
    raise_error("use of identifier before declaration");
  }
  return val.get_type();
}

paren_expr::paren_expr(expr *expression) { this->expression = expression; }

void paren_expr::dump_tree() { expression->dump_tree(); }

value paren_expr::codegen() { return expression->codegen(); }

type_i paren_expr::get_type() { return expression->get_type(); }

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
  return get_val(num, ir_builder.getInt32Ty());
}

value const_expr::get_val(int num, type_i type) {
  return get_val(num, type.llvm_type);
}

value const_expr::get_val(int num, llvm::Type *type) {
  value val;
  val.is_signed = true;
  auto *int_type = llvm::dyn_cast<llvm::IntegerType>(type);
  if (!int_type) {
    raise_error("Cannot get int value of non integer type");
  }
  val.llvm_val = llvm::ConstantInt::get(int_type, num, true);
  return val;
}

void const_expr::dump_tree() { cout << "- (constant) " << str << endl; }

value const_expr::codegen() {
  value ret_val;
  ret_val.llvm_val = data;
  ret_val.is_signed = is_signed;
  return ret_val;
}

type_i const_expr::get_type() { return type_i(data->getType(), is_signed); }

string_expr::string_expr(const char *c_str) : str(c_str) {
  free((void *)c_str);
  str = str.substr(1, str.length() - 2);  // Remove quotations
  unescape_escape_seqs();
}

void string_expr::dump_tree() { cout << "- (string) " << str << endl; }

value string_expr::codegen() {
  llvm::GlobalVariable *llvm_str = ir_builder.CreateGlobalString(str);
  value zero = const_expr::get_val(0);
  std::vector<llvm::Value *> indices{zero.llvm_val, zero.llvm_val};
  llvm::Value *ptr = ir_builder.CreateInBoundsGEP(llvm_str, indices);
  return value(ptr, true);
}

// Originally from https://stackoverflow.com/a/28766726/5585431
// Only supports a few sequences for now. Adding other simple escape sequences
// requires adding a case in the switch statement.
void string_expr::unescape_escape_seqs() {
  auto &s = str;
  std::stringstream ss{""};

  for (size_t i = 0; i < s.length(); i++) {
    if (s[i] == '\\') {
      switch (s[++i]) {
        case 'n':
          ss << "\n";
          break;
        case '"':
          ss << "\"";
          break;
        case '\\':
          ss << "\\";
          break;
        case '?':
          ss << "\?";
          break;
        case 't':
          ss << "\t";
          break;
        case 'r':
          ss << "\r";
          break;
        default:
          ss << "\\";
          i--;
      }
    } else {
      ss << s[i];
    }
  }

  str = ss.str();
}