# cc

A simple C compiler

The C language grammar (c.y and c.l files) have been taken from
[here](http://www.quut.com/c/ANSI-C-grammar-y-2011.html)

Using a C11 working draft, available
[here](http://www.open-std.org/JTC1/SC22/WG14/www/docs/n1570.pdf)

## Running

This project uses the LLVM C++ API, so it needs to be present on the system for
compilation. Version 11.0.0 has been used for development, but it should work on
lower versions too. Other libraries which need to be present are Flex and Bison
(used for lexing and parsing C files); and popt (used for parsing command line
args, should be present on most systems already)

We use CMake for building this project. In the repository's root directory, run
the following commands to compile:

```sh
cmake .
make
```

The `cc` binary is generated in the same directory. To compile a file `prog.c` to
LLVM IR file `prog.ll`, run the following:

```sh
./cc prog.c -o=prog
```

There are more flags available, run `./cc --help` for more information.

### Running tests

There are some test `.c` files in the tests/ directory which can be used to
test the compiler. The method used for testing is that these files are compiled
through both cc and a compiler on the system, and their output is compared.
These tests are run through CTest which comes with CMake. To build and run all tests, run:

```sh
make run_tests
```

**NOTE**: It is recommended to supply `make` with `-j8` (or any other appropriate
number) so it can utilise multiple threads which makes compilation and running
tests much faster

## Select supported constructs

- All integer types such as int, long, long long, etc. along with signed and
  unsigned specifiers
- Implicit type casting between integer types
- `if` and `switch`, `while` and `do-while`
- Use of `break` and `continue` wherever applicable
- `goto` and `return` statements
- Global and local variable declaration (with initializer support)

## Supported optimizations

For now, the compiler supports some local optimizations which are listed:

- Constant folding
- Dead code removal: Some optimizations supported under this would include not
  generating the `while` body if the condition is constant and evaluates to zero.
  Similarly for the `if` statement.

### Limitations

One major limitation of dead code removal is the presence of labeled statements,
since a `goto` can the jump into the body of a `while` loop for which we didn't
generate code which causes problems. Currently, if the while loop body has any
labeled statement, the entire body is always generated. This can be improved in
with better bookkeeping by seeing which labels are jumped into, etc. Note that a
similar issue also occurs with having a constant condition in the `if` statement.

## All supported constructs

Different C constructs' implementation has been put in different files, listed
below:

- decl_common.hpp
  - Storage-class specifiers
  - Type specifiers (currently not supporting atomic, struct, union, enum,
    typedef)
  - Declaration specifiers
  - Pointer
- declarator.hpp
  - Direct Declarator
    - Function Declarator
  - Declarator
  - Parameter list (Using same class for Parameter Type List)
  - Parameter Declaration
- expression.hpp
  - Expression
  - Assignment Expression
  - Binary Expression (Multiplicative Expression through Logical Or Expression)
  - Conditional Expression
  - Unary Expression
    - Unary operators (does not support `*` and `&` operators)
    - Increment and Decrement
  - Postfix Expression
    - Increment and Decrement
    - Function Call
  - Primary Expression
    - Identifier
    - Constant (only Integer Constant)
    - Parenthesized Expression
    - String
- func_def.hpp
  - External Declaration
  - Function Definition
  - Translation Unit
- block_item.hpp
  - Block Item
- statement.hpp
  - Statement
  - Labeled Statement
    - goto corresponding labeled statements
    - case and default
  - Compound Statement
  - Expression Statement
  - Selection Statements
    - `if`
    - `switch`
  - Iteration Statements
    - `while`
    - `do while`
  - Jump Statements
    - `goto`
    - `continue`
    - `break`
    - `return`

Other files' use has been given below:

- printer.hpp

  Extends `std::cout` to allow indented printing. Keeps track of the
  indentation level and prints each new line according to that. A caveat is that
  it only considers a new line when `std::endl` is provided to it.
- ast.hpp

  Contains the `ast_node` class which is a base class for all other
  classes which are a part of an AST. Statically holds a `printer` object and a
  pointer to the `base` node of the AST generated.

- symbol_table.hpp

  Contains implementation of the symbol table. Takes care of function scopes as
  well as block scopes