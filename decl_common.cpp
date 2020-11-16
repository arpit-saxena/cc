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

type_specifiers::type_specifiers(type spec) { add_spec(spec); }

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
    case VOID:
    case SCHAR:
    case UCHAR:
    case UINT:
    case FLOAT:
    case BOOL:
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
  }
}

type_specifiers::type type_specifiers::get_specs() { return specs; }

declaration_specs *declaration_specs::add(storage_specifiers::storage spec) {
  this->storage_spec.add_spec(spec);
  return this;
}

declaration_specs *declaration_specs::add(type_specifiers::type spec) {
  this->type_spec.add_spec(spec);
  return this;
}