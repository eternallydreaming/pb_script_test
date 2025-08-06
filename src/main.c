#include "bytecode.h"
#include "parser.h"
#include "value.h"
#include "vm.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  Chunk chunk = compile_script("4 == 4 && 5 > 2");
  disassemble_chunk(&chunk);
  Vm vm = new_vm(&chunk);
  Value result = run_vm(&vm);
  switch (result.type) {
  case ValueType_Null:
    printf("result: null\n");
    break;
  case ValueType_Number:
    printf("result: %f\n", result.number);
    break;
  case ValueType_Boolean:
    printf("result: %s\n", (result.boolean) ? "true" : "false");
    break;
  }
  return 0;
}