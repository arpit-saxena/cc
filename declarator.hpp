#ifndef DECLARATOR_HPP
#define DECLARATOR_HPP

#include <vector>

#include "ast.hpp"
#include "decl_common.hpp"

class direct_declarator : public ast_node {};

class declarator : public direct_declarator {
  pointer *p;  // optional
  direct_declarator *decl;

 public:
  declarator(direct_declarator *decl, pointer *p = nullptr);
};

class identifier_declarator : public direct_declarator {
  std::string identifier;

 public:
  identifier_declarator(std::string &&identifier);
};

////////////////// ARRAY DECLARATOR ///////////////////////

class array_declarator : public direct_declarator {
  direct_declarator *decl;

 public:
  array_declarator(direct_declarator *decl);
};

//////////////// FUNCTION DECLARATOR ///////////////////////

class param_declaration : public ast_node {
  declaration_specs *decl_spec;
  declarator *decl;  // Optional
  // TODO: Add option for abstract_declarator. Might be worth it to be break the
  // class into 2 classes and make this virtual

 public:
  param_declaration(declaration_specs *decl_spec, declarator *decl = nullptr);
};

class param_list : public ast_node {
  std::vector<param_declaration *> param_decls;
  bool is_vararg = false;

 public:
  param_list *add(param_declaration *decl);
  param_list *make_vararg();
};

class function_declarator : public direct_declarator {};

#endif /* DECLARATOR_HPP */