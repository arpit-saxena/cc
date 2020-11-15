#pragma once

#include <iostream>
#include <string>
#include <vector>

class ast_node {
 public:
  virtual std::string dump_tree();
  static void raise_error(std::string err);
};

/**
 * Section 6.7
 * Storage-class specifiers: Can be more than one but since no plan to support
 * _Thread_local, we can take it to be just one storage-class specifier
 * Type specifiers: Need to support use of multiple type specifiers
 */
/* class decl_specifiers_node : public ast_node {
  storage_specifiers storage = storage_specifiers::UNSET;
  basic_type_spec type =
      basic_type_spec::UNSET;  // TODO: Use of multiple type specifiers
 public:
  decl_specifiers_node *add_storage_spec(storage_specifiers spec);
  decl_specifiers_node *add_type_spec(basic_type_spec type);
};

class declarator_node : public ast_node {};

class func_def_node : public ast_node {};

class func_decl_node : public declarator_node {
  std::string name;

 public:
  func_decl_node() {}
};

class param_type_list_node : public ast_node {};

class param_decl_node : public ast_node {
  decl_specifiers_node *specifiers;
  declarator_node *decl;

 public:
  param_decl_node(ast_node *specifiers, ast_node *decl);
};

class param_list_node : public param_type_list_node {
  std::vector<param_decl_node *> param_decls;

 public:
  param_list_node(param_decl_node *decl);
  param_list_node *add_decl(param_decl_node *decl);
}; */