# cc

C compiler

The C language grammar (c.y and c.l files) have been taken from
[here](http://www.quut.com/c/ANSI-C-grammar-y-2011.html)

Using a C11 working draft, available
[here](http://www.open-std.org/JTC1/SC22/WG14/www/docs/n1570.pdf)

---

Different C constructs' implementation has been put in different files, listed
below:

- decl_common.hpp
  - Storage-class specifiers
  - Type specifiers (currently not supporting atomic, struct, union, enum,
    typedef)
  - Declaration specifiers
- declarator.hpp
  - Direct Declarator
    - Function Declarator
  - Declarator
  - Parameter list (Using same class for Parameter Type List)
  - Parameter Declaration
- expression.hpp
  - Expression
  - Binary Expression (Multiplicative Expression through Logical Or Expression)
  - Primary Expression
    - Identifier
    - Constant (only Integer Constant)
    - Parenthesized Expression
- func_def.hpp
  - External Declaration
  - Function Definition
  - Translate Unit
- statement.hpp
  - Block Item
  - Statement
  - Compound Statement
  - Expression Statement

Other files' use has been given below:

- printer.hpp

  Extends `std::cout` to allow indented printing. Keeps track of the
  indentation level and prints each new line according to that. A caveat is that
  it only considers a new line when `std::endl` is provided to it.
- ast.hpp

  Contains the `ast_node` class which is a base class for all other
  classes which are a part of an AST. Statically holds a `printer` object and a
  pointer to the `base` node of the AST generated.
