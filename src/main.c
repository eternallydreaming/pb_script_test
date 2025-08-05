#include "bytecode.h"
#include "lexer.h"
#include "parser.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  Chunk chunk = compile_script("5 * 5 - 20");
  disassemble_chunk(&chunk);
  Vm vm = new_vm(&chunk);
  double result = run_vm(&vm);
  printf("result: %f\n", result);
  return 0;
}