#ifndef DECLARATOR_HPP
#define DECLARATOR_HPP

#include <vector>

#include "ast.hpp"
#include "decl_common.hpp"

class direct_decl : public ast_node {};

class declarator_node : public direct_decl {
  pointer_node *p;  // optional
  direct_decl *decl;

 public:
  declarator_node(direct_decl *decl, pointer_node *p = nullptr);
};

class identifier_declarator : public direct_decl {
  std::string identifier;

 public:
  identifier_declarator(std::string &&identifier);
};

////////////////// ARRAY DECLARATOR ///////////////////////

class array_declarator : public direct_decl {
  direct_decl *decl;

 public:
  array_declarator(direct_decl *decl);
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
};

class param_list : public ast_node {
  std::vector<param_declaration *> param_decls;
  bool is_vararg = false;

 public:
  param_list *add(param_declaration *decl);
  param_list *make_vararg();
};

class function_declarator : public direct_decl {
  direct_decl *decl;
  param_list *params;  // Optional

 public:
  function_declarator(direct_decl *decl, param_list *params = nullptr);
  static void old_style_error();
};

#endif /* DECLARATOR_HPP */