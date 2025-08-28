#include "bytecode.h"
#include "parser.h"
#include "registry.h"
#include "type_def.h"
#include "value.h"
#include "vm.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

Value script_print(Vm *vm, size_t argc, Value argv[]) {
  for (size_t i = 0; i < argc; i++) {
    Value arg = argv[i];
    switch (arg.type) {
    case ValueType_Error:
    case ValueType_Void:
      assert(0);
    case ValueType_Null:
      fputs("null", stdout);
      break;
    case ValueType_Number:
      printf("%g", arg.number);
      break;
    case ValueType_Boolean:
      fputs(arg.boolean ? "true" : "false", stdout);
      break;
    case ValueType_String:
      fputs(arg.string->chars, stdout);
      break;
    }
    if (i != argc)
      putchar(' ');
  }
  putchar('\n');
  return new_null_value();
}

Value script_append(Vm *vm, size_t argc, Value argv[]) {
  return value_concat(argv[0], argv[1]);
}

Value script_tostring(Vm *vm, size_t argc, Value argv[]) {
  char result[128];
  int len = snprintf(result, sizeof(result), "%g", value_as_number(argv[0]));
  return new_string_value(result, len);
}

Value script_tonumber(Vm *vm, size_t argc, Value argv[]) {
  return new_number_value(atof(value_as_c_string(argv[0])));
}

Value script_check_number(Vm *vm, size_t argc, Value argv[]) {
  if (argv[0].type == ValueType_Number)
    printf("got number %g\n", argv[0].number);
  else
    puts("got null!!!");
  return new_null_value();
}

Value script_maybe_roll(Vm *vm, size_t argc, Value argv[]) {
  if (rand() % 2 == 0)
    return new_number_value(777);
  return new_null_value();
}

int main(int argc, char *argv[]) {
  srand(time(NULL));
  char source[1024];
  size_t len = fread(source, 1, sizeof(source) - 1, stdin);
  source[len] = 0;

  Registry registry = new_registry();
  register_native_fn(&registry, "void print(...)", script_print);
  register_native_fn(&registry, "string append(string, string)", script_append);
  register_native_fn(&registry, "string tostring(number)", script_tostring);
  register_native_fn(&registry, "number tonumber(string)", script_tonumber);
  register_native_fn(&registry, "void check_number(number?)",
                     script_check_number);
  register_native_fn(&registry, "number? maybe_roll()", script_maybe_roll);

  Chunk chunk = compile_script(source, &registry);
  disassemble_chunk(&chunk, &registry);
  Vm vm = new_vm(&chunk, &registry);
  run_vm(&vm);
  printf("alive values: %u\n", get_active_values());
  return 0;
}