#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  Token *tokens = tokenize_script("--5");
  compile_script(tokens);
  free(tokens);
  return 0;
}