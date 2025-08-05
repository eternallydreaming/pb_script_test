#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  compile_script("5 + 5 / 2");
  return 0;
}