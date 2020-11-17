#ifndef PRINTER_HPP
#define PRINTER_HPP

#include <iostream>
#include <ostream>

using std::endl;

// Template implementation from https://stackoverflow.com/a/25615296/5585431
class printer {
  const int spaces_indent;
  std::ostream &os;
  int indent_level = 0;
  bool is_newline = true;

  void print_indent();

 public:
  printer(int indent = 4, std::ostream &os = std::cout)
      : spaces_indent{indent}, os{os} {}

  void indent();
  void unindent();

  template <typename T>
  printer &operator<<(T &&obj) {
    if (is_newline) {
      is_newline = false;
      print_indent();
    }
    os << std::forward<T>(obj);
    return *this;
  }

  printer &operator<<(std::ostream &(*manip)(std::ostream &));
};

#endif /* PRINTER_HPP */