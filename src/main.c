#include "bytecode.h"
#include "parser.h"
#include "value.h"
#include "vm.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  char source[1024];
  size_t len = fread(source, 1, sizeof(source) - 1, stdin);
  source[len] = 0;

  Chunk chunk = compile_script(source);
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
  case ValueType_String:
    printf("result: \"%s\"\n", result.string->chars);
    break;
  }
  printf("alive values: %u\n", get_active_values());
  return 0;
}