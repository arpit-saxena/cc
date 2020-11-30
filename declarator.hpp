#ifndef DECLARATOR_HPP
#define DECLARATOR_HPP

#include <llvm/IR/Type.h>

#include <vector>

#include "ast.hpp"
#include "decl_common.hpp"

class direct_decl : public ast_node {
 public:
  virtual void dump_tree();
};

class declarator_node : public direct_decl {
  pointer_node *p;  // optional
  direct_decl *decl;

 public:
  declarator_node(direct_decl *decl, pointer_node *p = nullptr);
  void dump_tree() override;
};

class identifier_declarator : public direct_decl {
  std::string identifier;

 public:
  identifier_declarator(std::string &&identifier);
  void dump_tree() override;
};

////////////////// ARRAY DECLARATOR ///////////////////////

class array_declarator : public direct_decl {
  direct_decl *decl;

 public:
  array_declarator(direct_decl *decl);
  void dump_tree() override;
};

//////////////// FUNCTION DECLARATOR ///////////////////////

class param_declaration : public ast_node {
  declaration_specs *decl_spec;
  declarator_node *decl;  // Optional
  // TODO: Add option for abstract_declarator. Might be worth it to be break the
  // class into 2 classes and make this virtual

 public:
  param_declaration(declaration_specs *decl_spec,
                    declarator_node *decl = nullptr);
  void dump_tree() override;
  llvm::Type *get_type();
};

class param_list : public ast_node {
  std::vector<param_declaration *> param_decls;
  bool is_vararg = false;

 public:
  param_list *add(param_declaration *decl);
  param_list *make_vararg();
  void dump_tree() override;
  std::vector<llvm::Type *> get_types();
};

class function_declarator : public direct_decl {
  direct_decl *decl;
  param_list *params;  // Optional

 public:
  function_declarator(direct_decl *decl, param_list *params = nullptr);
  static void old_style_error();
  void dump_tree() override;
};

#endif /* DECLARATOR_HPP */