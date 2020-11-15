#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.hpp"
#include "decl_common.hpp"

extern "C" int yylex();
#include "c.tab.hpp"
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
  int ret = yyparse();
  printf("retv = %d\n", ret);
  exit(0);
}
