#ifndef FUNC_DEF_HPP
#define FUNC_DEF_HPP

#include "ast.hpp"
#include "decl_common.hpp"
#include "declarator.hpp"
#include "statement.hpp"

class external_decl : public ast_node {};

class func_def : public external_decl {
  declaration_specs *decl_specs;
  declarator_node *declarator;
  compound_stmt *statement;

 public:
  func_def(declaration_specs *decl_specs, declarator_node *declarator,
           compound_stmt *statement);
  static void old_style_error();
};

class trans_unit : public ast_node {
  std::vector<external_decl *> external_decls;

 public:
  trans_unit *add(external_decl *decl);
};

#endif /* FUNC_DEF_HPP */