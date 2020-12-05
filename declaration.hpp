#ifndef DECLARATION_HPP
#define DECLARATION_HPP

#include <vector>

#include "ast.hpp"
#include "block_item.hpp"
#include "declarator.hpp"
#include "expression.hpp"
#include "func_def.hpp"

class initializer_node : public ast_node {
 public:
  virtual void dump_tree() override;
  virtual value codegen() = 0;
};

// TODO: Fix this. Ideally this class shouldn't exist but multiple inheritance
// is causing issues the base case not being pure virtual.
class init_assign_expr : public initializer_node {
  assign_expr *expression;

 public:
  init_assign_expr(assign_expr *expression);
  void dump_tree() override;
  value codegen() override;
};

// Not using designation for now
class initializer_lst : public initializer_node {
  std::vector<initializer_node *> inits;

 public:
  initializer_lst *add(initializer_node *init);
  void dump_tree() override;
  value codegen() override;
};

class init_decl : public ast_node {
  declarator_node *declarator;
  initializer_node *initializer;  // Optional

 public:
  init_decl(declarator_node *declarator,
            initializer_node *initializer = nullptr);
  void dump_tree() override;
  value codegen(type_i type);
};

class init_decl_list : public ast_node {
  std::vector<init_decl *> declarators;

 public:
  init_decl_list *add(init_decl *declarator);
  void dump_tree() override;
  std::vector<value> codegen(type_i type);
};

// Ignoring static_assert_declaration
class declaration_node : public blk_item, public external_decl {
  declaration_specs *specs;
  init_decl_list *decl_list;  // Optional

  // 6.7/2 says that all declarations need to declare at least a declarator,
  // a tag, or the members of an enumeration barring some exceptions.
  // Since we don't have support for tags or enums, the rule boils down to
  // choice of having declarator, which is controlled by this variable
  bool allow_no_declarator = false;

 public:
  declaration_node(declaration_specs *specs,
                   init_decl_list *decl_list = nullptr);
  void dump_tree() override;
  virtual void codegen() override;
};

#endif /* DECLARATION_HPP */