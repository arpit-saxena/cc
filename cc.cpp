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
  int no_codegen = 0;
  char *t;
  poptOption options_table[] = {
      {"print-ast", 'p', POPT_ARG_NONE, &print_ast, 0,
       "Prints generated ast. Also disables code generation", "print ast"},
      {"no-codegen", 'n', POPT_ARG_NONE, &no_codegen, 0,
       "Don't generate code. Only AST is parsed and any parsing errors are "
       "reported",
       "Don't generate code"},
      {nullptr, 'o', POPT_ARG_STRING, &out_file_name, 0,
       "Base name of output .ll file", "outfile-basename"},
      POPT_AUTOHELP POPT_TABLEEND};

  poptContext context = poptGetContext(nullptr, argc, argv, options_table, 0);

  while (poptGetNextOpt(context) >= 0) {
  }

  const char *filename = poptGetArg(context);

  if (!filename) {
    printf("Specify a filename after command\n");
    return 1;
  }

  yyin = fopen(filename, "r");

  if (!yyin) {
    printf("Unable to open file \"%s\"\n", filename);
    return 2;
  }

  assert(yyin);

  // yydebug = 1;
  int ret = yyparse();
  if (print_ast) {
    printf("--- AST BEGIN ---\n\n");
    ast_node::base->dump_tree();
    printf("\n--- AST END ---\n\n");
  }

  if (ret != 0) {
    printf("Parse failed. No output file written\n");
    exit(2);
  }

  if (!no_codegen && !print_ast) {
    std::string out_str;
    if (!out_file_name) {
      out_str = "a.ll";
    } else {
      out_str = out_file_name;
      out_str += ".ll";
    }

    ast_node::base->codegen();

    std::error_code err;
    llvm::raw_fd_ostream file(out_str, err);
    if (err) {
      printf("An error occurred in opening file: %s\n", err.message().c_str());
      return 2;
    }
    ast_node::the_module->print(file, nullptr);
  }
}
