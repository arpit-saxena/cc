#include <assert.h>
#include <popt.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.hpp"
#include "c.tab.hpp"

extern "C" int yylex();
int yyparse();
extern "C" FILE *yyin;

void usage(poptContext context) { poptPrintUsage(context, stderr, 0); }

int main(int argc, const char **argv) {
  int print_ast = 0;
  char *out_file_name = nullptr;
  poptOption options_table[] = {
      {"print-ast", 'p', POPT_ARG_NONE, &print_ast, 0, "print ast",
       "print ast"},
      {nullptr, 'o', POPT_ARG_STRING, &out_file_name, 0,
       "base name of output .ll file", "outfile-basename"},
      POPT_AUTOHELP POPT_TABLEEND};

  poptContext context = poptGetContext(nullptr, argc, argv, options_table, 0);

  while (poptGetNextOpt(context) >= 0) {
  }

  const char *filename = poptGetArg(context);

  if (!filename) {
    printf("Specify a filename after command\n");
    exit(1);
  }

  yyin = fopen(filename, "r");
  assert(yyin);

  // yydebug = 1;
  int ret = yyparse();
  if (print_ast) {
    printf("--- AST BEGIN ---\n\n");
    ast_node::base->dump_tree();
    printf("\n--- AST END ---\n\n");
  }

  printf("Parse retv = %d\n", ret);

  std::string out_str;
  if (!out_file_name) {
    out_str = "a.ll";
  } else {
    out_str = out_file_name;
    out_str += ".ll";
  }

  printf("\nGenerating code...\n");
  ast_node::base->codegen();
  printf("Generated, written to %s\n", out_str.c_str());

  std::error_code err;
  llvm::raw_fd_ostream file(out_str, err);
  if (err) {
    printf("An error occurred in opening file: %s\n", err.message().c_str());
    return 2;
  }
  ast_node::the_module->print(file, nullptr);
  exit(0);
}
