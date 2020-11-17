#include "printer.hpp"

void printer::print_indent() {
  for (int i = 0; i < indent_level; i++) {
    for (int j = 0; j < spaces_indent; j++) {
      os << ' ';
    }
  }
}

void printer::indent() { indent_level++; }

void printer::unindent() { indent_level--; }

// FIXME: Currently assuming only manipulator given is endl
printer &printer::operator<<(std::ostream &(*manip)(std::ostream &)) {
  os << manip;
  is_newline = true;
  return *this;
}