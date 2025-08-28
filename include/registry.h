#pragma once

#include "type_def.h"
#include "value.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct Vm;
typedef Value (*NativeFnPtr)(struct Vm *vm, size_t argc, Value argv[]);

typedef struct NativeFn {
  TypeDef return_type;
  char *name;

  size_t args_num;
  TypeDef *arg_types;
  bool variadic;

  NativeFnPtr ptr;
} NativeFn;

typedef struct Registry {
  size_t native_fns_num;
  NativeFn *native_fns;
} Registry;

Registry new_registry();
void delete_registry(Registry *registry);

void register_native_fn(Registry *registry, const char *sig, NativeFnPtr ptr);