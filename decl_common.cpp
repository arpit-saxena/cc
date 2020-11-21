#include "decl_common.hpp"

#include <string>

storage_specifiers::storage_specifiers(storage spec) { this->spec = spec; }

storage_specifiers *storage_specifiers::add_spec(storage spec) {
  if (this->spec != UNSET) {
    raise_error("Only one storage specifier is allowed. Taking the new one!");
  }

  this->spec = spec;
  return this;
}

void storage_specifiers::dump_tree() {
  if (spec != UNSET) {
    cout << "- (storage_specifiers) " << to_string() << endl;
  }
}

std::string storage_specifiers::to_string() {
  switch (spec) {
    case UNSET:
      return "unset";
    case TYPEDEF:
      return "typedef";
    case EXTERN:
      return "extern";
    case STATIC:
      return "static";
    case THREAD_LOCAL:
      return "_Thread_local";
    case AUTO:
      return "auto";
    case REGISTER:
      return "register";
  }
  return "unkown";
}

type_specifiers::type_specifiers(type spec) {
  if (spec == UNSET) {
    this->specs = UNSET;
    return;
  }
  add_spec(spec);
}

type_specifiers *type_specifiers::raise_incompat_error(type spec) {
  raise_error("Incompatible type specifiers");
  return this;
}

type_specifiers *type_specifiers::change_type(type specs) {
  this->specs = specs;
  return this;
}

type_specifiers *type_specifiers::add_spec(type spec) {
  bool was_inserted = types_seen.insert(spec).second;
  // FIXME maybe: Weirdish fix for LONG being able to appear twice
  if (!was_inserted && spec == LONG) {
    was_inserted = types_seen.insert(LONG_LONG).second;
  }
  if (!was_inserted) {  // => Already present
    return raise_incompat_error(spec);
  }

  switch (this->specs) {
    case UNSET:
      return change_type(spec);
    case VOID:
    case SCHAR:
    case UCHAR:
    case UINT:
    case FLOAT:
    case BOOL:
    case LONG_DOUBLE:
      return raise_incompat_error(spec);
    case CHAR:
      switch (spec) {
        case UNSIGNED:
          return change_type(UCHAR);
        case SIGNED:
          return change_type(SCHAR);
        default:
          return raise_incompat_error(spec);
      }
    case SHORT:
      switch (spec) {
        case SIGNED:
        case INT:
          return this;
        case UNSIGNED:
          return change_type(USHORT);
        default:
          return raise_incompat_error(spec);
      }
    case USHORT:
      switch (spec) {
        case INT:
          return this;
        default:
          return raise_incompat_error(spec);
      }
    case INT:
      switch (spec) {
        case SHORT:
          return change_type(SHORT);
        case LONG:
          return change_type(LONG);
        case SIGNED:
          return this;
        case UNSIGNED:
          return change_type(UINT);
        default:
          return raise_incompat_error(spec);
      }
    case LONG:
      switch (spec) {
        case SIGNED:
        case INT:
          return this;
        case LONG:
          return change_type(LONG_LONG);
        case UNSIGNED:
          return change_type(ULONG);
        case DOUBLE:
          return change_type(LONG_DOUBLE);
        default:
          return raise_incompat_error(spec);
      }
    case ULONG:
      switch (spec) {
        case INT:
          return this;
        case LONG:
          return change_type(ULONG_LONG);
        default:
          return raise_incompat_error(spec);
      }
    case LONG_LONG:
      switch (spec) {
        case INT:
        case SIGNED:
          return this;
        case UNSIGNED:
          return change_type(ULONG_LONG);
        default:
          return raise_incompat_error(spec);
      }
    case ULONG_LONG:
      switch (spec) {
        case INT:
          return this;
        default:
          return raise_incompat_error(spec);
      }
    case DOUBLE:
      switch (spec) {
        case LONG:
          return change_type(LONG_DOUBLE);
        default:
          return raise_incompat_error(spec);
      }
    case SIGNED:
      switch (spec) {
        case INT:
        case SHORT:
        case LONG:
        case LONG_LONG:
          return change_type(spec);
        case CHAR:
          return change_type(SCHAR);
        default:
          return raise_incompat_error(spec);
      }
    case UNSIGNED:
      switch (spec) {
        case INT:
          return change_type(UINT);
        case SHORT:
          return change_type(USHORT);
        case LONG:
          return change_type(ULONG);
        case LONG_LONG:
          return change_type(ULONG_LONG);
        case CHAR:
          return change_type(UCHAR);
        default:
          return raise_incompat_error(spec);
      }
  }

  raise_error("Undefined type given to type_specifiers::add_spec");
  return this;
}

type_specifiers::type type_specifiers::get_specs() { return specs; }

void type_specifiers::dump_tree() {
  cout << "- (type_specifier) " << to_string() << endl;
}

std::string type_specifiers::to_string() {
  switch (specs) {
    case UNSET:
      return "unset";
    case VOID:
      return "void";
    case CHAR:
      return "char";
    case SCHAR:
      return "signed char";
    case UCHAR:
      return "unsigned char";
    case SHORT:
      return "short";
    case USHORT:
      return "unsigned short";
    case INT:
      return "int";
    case UINT:
      return "unsigned int";
    case LONG:
      return "long";
    case ULONG:
      return "unsigned long";
    case LONG_LONG:
      return "long long";
    case ULONG_LONG:
      return "unsigned long long";
    case FLOAT:
      return "float";
    case DOUBLE:
      return "double";
    case LONG_DOUBLE:
      return "long double";
    case SIGNED:
      return "signed";
    case UNSIGNED:
      return "unsigned";
    case BOOL:
      return "bool";
  }
  return "unknown";
}

type_qualifiers *type_qualifiers::add_qual(type_qualifiers::qualifier qual) {
  // Section 6.7.3, point 5: "If the same qualifier appears more than once
  // in the samespecifier-qualifier-list, either directly or via one or
  // more typedefs, the behavior is the same as if it appeared only once."
  // So we simply insert into the set and don't check for repetitions;
  qualifiers.insert(qual);
  return this;
}

void type_qualifiers::dump_tree() {
  if (qualifiers.empty()) {
    return;
  }

  cout << "- (type_qualifiers) ";
  for (auto qual : qualifiers) {
    cout << to_string(qual) << ' ';
  }
  cout << endl;
}

std::string type_qualifiers::to_string(type_qualifiers::qualifier qual) {
  switch (qual) {
    case CONST:
      return "const";
    case RESTRICT:
      return "restrict";
    case VOLATILE:
      return "volatile";
    case ATOMIC:
      return "_Atomic";
  }
  return "unknown";
}

declaration_specs *declaration_specs::add(storage_specifiers::storage spec) {
  this->storage_spec.add_spec(spec);
  return this;
}

declaration_specs *declaration_specs::add(type_specifiers::type spec) {
  this->type_spec.add_spec(spec);
  return this;
}

declaration_specs *declaration_specs::add(type_qualifiers::qualifier qual) {
  this->type_qual.add_qual(qual);
  return this;
}

void declaration_specs::dump_tree() {
  cout << "- (declaration_specifiers)" << endl;
  cout.indent();
  storage_spec.dump_tree();
  type_spec.dump_tree();
  type_qual.dump_tree();
  cout.unindent();
}