#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <llvm/IR/Value.h>

#include <string>
#include <utility>

#include "ast.hpp"

/*
 * There is a class hierarchy of empty classes denoting a particular kind of
 * expression And for a class actually representing that kind of expression, the
 * class is derived from that expression with "_ops" appended, meaning the
 * expression with operands.
 *
 * This is done so an expression down the hierarchy don't have to construct all
 * expressions up the hierarchy. It also makes logical sense since, for example
 * a conditional expression is an assignment expression, but "a = 5" is an
 * assignment expression and not a conditional expression
 */

class expr : public ast_node {
 public:
  virtual value codegen() {
    raise_error("codegen not implemented for this expression!");
  };  // TODO: Make this pure virtual
  virtual type_i get_type() {
    raise_error("get_type not implemented for this expression!");
  }  // TODO: Make this pure virtual
  static void convert_to_bool(llvm::Value *&val);
  static void convert_to_type(value &val, type_i type);
  static void gen_common_type(value &lhs, value &rhs);
  static type_i get_common_type(type_i type1, type_i type2);
  static type_i get_common_type(expr *type1, expr *type2);
};

// When a value needs to be used as an expression
class value_expr : public expr {
  value val;

 public:
  value_expr(value v) : val(v) {}
  value codegen() override { return val; }
  void dump_tree() override { cout << "- (value)" << endl; }
  type_i get_type() override { return val.get_type(); }
};

class assign_expr : public expr {};

class unary_expr;  // forward declaration

class assign_expr_ops : public assign_expr {
 public:
  enum OP {
    ASSIGN,        // '='
    MUL_ASSIGN,    // '*='
    DIV_ASSIGN,    // '/='
    MOD_ASSIGN,    // '%='
    ADD_ASSIGN,    // '+='
    SUB_ASSIGN,    // '-='
    LEFT_ASSIGN,   // '<<='
    RIGHT_ASSIGN,  // '>>='
    AND_ASSIGN,    // '&='
    XOR_ASSIGN,    // '^='
    OR_ASSIGN,     // '|='
  };

 private:
  unary_expr *left_expr;
  OP op;
  assign_expr *right_expr;

 public:
  assign_expr_ops(unary_expr *left_expr, OP op, assign_expr *right_expr);
  static std::string op_string(OP op);
  void dump_tree() override;
  value codegen() override;
};

class binary_expr;  // forward declaration

class cond_expr : public assign_expr {};

class cond_expr_ops : public cond_expr {
  binary_expr *cond;
  expr *true_expr;
  cond_expr *false_expr;

 public:
  cond_expr_ops(binary_expr *cond, expr *true_expr, cond_expr *false_expr);
  value codegen() override;
  // Assumes value to be i1
  static value codegen(value cond, expr *true_expr, expr *false_expr);
  void dump_tree() override;
  type_i get_type() override;
};

class binary_expr : public cond_expr {};

// Rest of the expressions have been combined into one class
// If need be later, this can be broken down into the proper hierarchy
class binary_expr_ops : public binary_expr {
 public:
  enum OP {
    MULT,         // '*'
    DIV,          // '/'
    MOD,          // '%'
    PLUS,         // '+'
    MINUS,        // '-'
    SHIFT_LEFT,   // '<<'
    SHIFT_RIGHT,  // '>>'
    LT,           // '<'
    GT,           // '>'
    LE,           // '<='
    GE,           // '>='
    EQ,           // '=='
    NE,           // '!='
    BIT_AND,      // '&'
    BIT_XOR,      // '^'
    BIT_OR,       // '|'
    AND,          // '&&'
    OR,           // '||'
  };

 private:
  expr *left_expr;
  OP op;
  expr *right_expr;
  static llvm::Instruction::BinaryOps get_arith_op(value &lhs, value &rhs,
                                                   OP op);
  static llvm::CmpInst::Predicate get_cmp_pred(value &lhs, value &rhs, OP op);

 public:
  binary_expr_ops(expr *left, OP op, expr *right);
  static std::string op_string(OP op);
  virtual void dump_tree() override;
  value codegen() override;
  static value codegen(expr *left, OP op, expr *right);
  static value codegen(value left, OP op, value right);
  type_i get_type() override;
};

class cast_expr : public binary_expr {};

class unary_expr : public cast_expr {
 public:
  // For assignment expression for now. Might need to move this up the hierarchy
  // in future. This is essentially a workaround for not having a rich value
  // class. Ideally, it should know whether it is an lvalue or rvalue and how to
  // perform stores
  virtual value codegen_store(value val) {
    raise_error("Codegen store not defined for expression");
    // Ideally should be pure virtual but then will have to define this for all
    // classes down the hierarchy.
    // TODO: Make pure virtual, define for all lower classes
  };
};

class unary_op_expr : public unary_expr {
 public:
  enum OP { ADDRESS_OF, INDIRECTION, PLUS, MINUS, BIT_NOT, NOT };

 private:
  cast_expr *expression;
  OP op;

 public:
  unary_op_expr(OP unary_op, cast_expr *expression);
  static std::string op_string(OP op);
  virtual void dump_tree() override;
  value codegen() override;
  static value codegen(OP op, value val);
  type_i get_type() override;
};

class arg_expr_list : public ast_node {
  std::vector<assign_expr *> exprs;

 public:
  arg_expr_list *add(assign_expr *expr);
  void dump_tree() override;
  std::vector<type_i> get_types();
  std::vector<value> codegen();
};

class postfix_expr : public unary_expr {};

class func_call : public postfix_expr {
  postfix_expr *func_expr;
  arg_expr_list *arg_list;  // Optional

 public:
  func_call(postfix_expr *func_expr, arg_expr_list *expr_list = nullptr);
  void dump_tree() override;
  value codegen() override;
  type_i get_type() override;
};

class primary_expr : public postfix_expr {};

class ident_expr : public primary_expr {
  std::string identifier;

 public:
  ident_expr(const char *id);
  void dump_tree() override;
  value codegen() override;
  value codegen_store(value val) override;
  type_i get_type() override;
};

class paren_expr : public primary_expr {
  expr *expression;

 public:
  paren_expr(expr *expression);
  void dump_tree() override;
  value codegen() override;
  type_i get_type() override;
};

class const_expr : public primary_expr {
  llvm::Constant *data;
  std::string str;
  bool is_signed;

 public:
  const_expr(llvm::Constant *data, bool is_signed, std::string str);
  // includes character literals
  static const_expr *new_int_expr(const char *str);
  static value get_val(int num);
  static value get_val(int num, type_i type);
  static value get_val(int num, llvm::Type *type);
  void dump_tree() override;
  value codegen() override;
  type_i get_type() override;
};

class string_expr : public primary_expr {
  std::string str;
  // Escape sequences such as \n become \\n due to escaping of \ during parsing.
  // This function reverts that
  // Currently only unescapes some simple escape sequences
  void unescape_escape_seqs();

 public:
  string_expr(const char *str);
  void dump_tree() override;
  value codegen() override;
};

#endif /* EXPRESSION_HPP */
