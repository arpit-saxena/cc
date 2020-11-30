#ifndef DECL_COMMON_HPP
#define DECL_COMMON_HPP

#include <llvm/IR/Type.h>

#include <set>

#include "ast.hpp"

/*
 * (Section 6.7.1)
 * Represents zero or more storage specifiers in the declaration specifiers
 *
 * Implementation restrictions:
 * Since there is no plan to support threads, THREAD_LOCAL won't be supported.
 * So removing THREAD_LOCAL implies there can be at most one storage specifier
 * the declaration specifiers, so only one variable has been used
 */
class storage_specifiers : public ast_node {
 public:
  enum storage {
    UNSET,  // To indicate no storage specifier has been given
    TYPEDEF,
    EXTERN,
    STATIC,
    THREAD_LOCAL,
    AUTO,
    REGISTER
  };

 private:
  storage spec;

 public:
  storage_specifiers(storage spec = UNSET);
  storage_specifiers* add_spec(storage spec);
  void dump_tree() override;
  std::string to_string();
};

/*
 * (Section 6.7.2)
 * Represents one or more type specifiers in the declaration specifiers
 *
 * Implementation restrictions:
 * - COMPLEX is not mandated and so not supported by this implementation
 * - Does not contain atomic, struct, union, enum types. Implemented as derived
 *   classes
 * - While there is always atleast one type specifier in the declaration
 *   specifiers, the UNSET enum is given so a type specifier can be added later
 */
class type_specifiers : public ast_node {
 public:
  enum type {
    UNSET,
    VOID,
    CHAR,
    SCHAR,
    UCHAR,
    SHORT,
    USHORT,
    INT,
    UINT,
    LONG,
    ULONG,
    LONG_LONG,
    ULONG_LONG,
    FLOAT,
    DOUBLE,
    LONG_DOUBLE,
    SIGNED,
    UNSIGNED,
    BOOL,
    /* Doesn't include complex (non mandated) */
    // TODO: Implement atomic, struct, union, enum as derived classes
  };

 private:
  std::set<type> types_seen;
  type specs;
  type_specifiers* raise_incompat_error(type spec);
  type_specifiers* change_type(type specs);

 public:
  type_specifiers(type spec = UNSET);
  type_specifiers* add_spec(type spec);
  type get_specs();
  void dump_tree() override;
  std::string to_string();
  llvm::Type* get_type();
};

/*
 * (Section 6.7.3)
 * Represents one or more type qualifiers in the declaration specifiers
 */
class type_qualifiers : public ast_node {
 public:
  enum qualifier { CONST, RESTRICT, VOLATILE, ATOMIC };

 private:
  std::set<qualifier> qualifiers;

 public:
  type_qualifiers* add_qual(qualifier qual);
  void dump_tree() override;
  static std::string to_string(qualifier qual);
};

// TODO: Add function_specifier and alignment_specifier here

class pointer_node : public ast_node {
  std::vector<type_qualifiers*> quals;

 public:
  pointer_node* add(type_qualifiers* quals);
  void dump_tree() override;
  llvm::PointerType* get_type(llvm::Type* type);
};

class declaration_specs : public ast_node {
  storage_specifiers storage_spec;
  type_specifiers type_spec;
  type_qualifiers type_qual;
  // TODO: Add function_specifier and alignment specifier

 public:
  declaration_specs* add(storage_specifiers::storage storage_spec);
  declaration_specs* add(type_specifiers::type type_spec);
  declaration_specs* add(type_qualifiers::qualifier type_qual);
  void dump_tree() override;
  llvm::Type* get_type();
};

#endif /* DECL_COMMON_HPP */