#ifndef DECLARATOR_HPP
#define DECLARATOR_HPP

#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

#include <string>
#include <vector>

#include "ast.hpp"
#include "decl_common.hpp"

class direct_decl : public ast_node {
 public:
  virtual void dump_tree();
  virtual std::string get_identifier() = 0;
};

class declarator_node : public direct_decl {
  pointer_node *p;  // optional
  direct_decl *decl;

 public:
  declarator_node(direct_decl *decl, pointer_node *p = nullptr);
  void dump_tree() override;
  std::string get_identifier() override;
  llvm::Function *gen_function(declaration_specs *specs);
};

class identifier_declarator : public direct_decl {
  std::string identifier;

 public:
  identifier_declarator(std::string &&identifier);
  void dump_tree() override;
  std::string get_identifier() override;
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
  // returns true if a name was set, false otherwise
  bool set_arg_name(llvm::Argument *arg);
};

class param_list : public ast_node {
  std::vector<param_declaration *> param_decls;
  bool varargs = false;

 public:
  param_list *add(param_declaration *decl);
  param_list *make_vararg();
  void dump_tree() override;
  std::vector<llvm::Type *> get_types();
  bool is_vararg();
  // Set names of arguments if present
  void set_arg_names(llvm::iterator_range<llvm::Argument *> args);
};

class function_declarator : public direct_decl {
  direct_decl *decl;
  param_list *params;  // Optional

 public:
  function_declarator(direct_decl *decl, param_list *params = nullptr);
  static void old_style_error();
  void dump_tree() override;
  std::string get_identifier() override;
  llvm::Function *get_function(llvm::Type *ret_type);
};

#endif /* DECLARATOR_HPP */