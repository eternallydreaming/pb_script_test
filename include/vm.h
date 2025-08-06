#pragma once

#include "chunk.h"
#include "value.h"
#include <stddef.h>

typedef struct Vm {
  const Chunk *chunk;
  Value stack[128];
  size_t sp;
  size_t pc;
} Vm;

Vm new_vm(const Chunk *chunk);

Value run_vm(Vm *vm);