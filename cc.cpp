#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.hpp"
#include "c.tab.hpp"

extern "C" int yylex();
int yyparse();
extern "C" FILE *yyin;

static void usage() { printf("Usage: cc <prog.c>\n"); }

int main(int argc, char **argv) {
  if (argc != 2) {
    usage();
    exit(1);
  }
  char const *filename = argv[1];
  yyin = fopen(filename, "r");
  assert(yyin);
  // yydebug = 1;
  int ret = yyparse();
  printf("--- AST BEGIN ---\n\n");
  ast_node::base->dump_tree();
  printf("\n--- AST END ---\n\n");
  printf("retv = %d\n", ret);
  exit(0);
}
