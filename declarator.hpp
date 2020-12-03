#ifndef DECLARATOR_HPP
#define DECLARATOR_HPP

#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

#include <string>
#include <vector>

#include "ast.hpp"
#include "decl_common.hpp"

class function_declarator;  // forward declaration

class direct_decl : public ast_node {
 public:
  virtual void dump_tree();
  virtual std::string get_identifier() = 0;
  // TODO: Make this pure virtual
  virtual void codegen(llvm::Type *type){};  // type is of the decl specifiers
  // Helper method to get a function declarator anywhere inside the declarator
  // hierarchy, would return nullptr if not found
  virtual function_declarator *get_func_decl() = 0;
};

class declarator_node : public direct_decl {
  pointer_node *p;  // optional
  direct_decl *decl;

 public:
  declarator_node(direct_decl *decl, pointer_node *p = nullptr);
  void dump_tree() override;
  std::string get_identifier() override;
  virtual function_declarator *get_func_decl() override {
    return decl->get_func_decl();
  }
  llvm::Type *get_type(llvm::Type *type);
  void codegen(llvm::Type *type) override;
};

class identifier_declarator : public direct_decl {
  std::string identifier;

 public:
  identifier_declarator(std::string &&identifier);
  void dump_tree() override;
  std::string get_identifier() override;
  function_declarator *get_func_decl() override { return nullptr; }

  // This basically generates an alloca for a variable of given type. So this
  // shouldn't be called when the declarator is part of a function
  void codegen(llvm::Type *type) override;
};

////////////////// ARRAY DECLARATOR ///////////////////////

class array_declarator : public direct_decl {
  direct_decl *decl;

 public:
  array_declarator(direct_decl *decl);
  void dump_tree() override;
  function_declarator *get_func_decl() override { return nullptr; }
  void codegen(llvm::Type *type) override;
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
  // Add mapping of identifier (if present) to given value in symbol table
  void add_arg_table(llvm::Value *value);
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
  // Add args mapping to top scope of symbol table
  void add_args_table(std::vector<llvm::Value *> values);
};

class function_declarator : public direct_decl {
  direct_decl *decl;
  param_list *params;  // Optional
  llvm::Function *codegen_common(llvm::Type *ret_type);

 public:
  function_declarator(direct_decl *decl, param_list *params = nullptr);
  static void old_style_error();
  void dump_tree() override;
  std::string get_identifier() override;
  virtual function_declarator *get_func_decl() override { return this; }

  // Generates the code for function definition. This would generate a function
  // and the corresponding arguments, and push them onto the symbol table in a
  // function scope
  llvm::Function *codegen_def(llvm::Type *ret_type);
};

#endif /* DECLARATOR_HPP */