#include "vm.h"
#include "bytecode.h"
#include "chunk.h"
#include "value.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

static uint8_t read_u8(Vm *vm) { return read_chunk_u8(vm->chunk, &vm->pc); }
static uint64_t read_u64(Vm *vm) { return read_chunk_u64(vm->chunk, &vm->pc); }
static double read_f64(Vm *vm) { return read_chunk_f64(vm->chunk, &vm->pc); }

static void push(Vm *vm, Value value) {
  assert(vm->sp < 128);
  vm->stack[vm->sp++] = value;
}

static Value pop(Vm *vm) {
  assert(vm->sp != 0);
  return vm->stack[--vm->sp];
}

Vm new_vm(const Chunk *chunk) {
  return (Vm){
      .chunk = chunk,
      .stack = {},
      .sp = 0,
      .pc = 0,
  };
}

double run_vm(Vm *vm) {
#define BINARY_OP(op)                                                          \
  do {                                                                         \
    double rhs = value_as_number(pop(vm));                                     \
    double lhs = value_as_number(pop(vm));                                     \
                                                                               \
    push(vm, new_number_value(lhs op rhs));                                    \
  } while (0)

  while (vm->pc < vm->chunk->size) {
    Bytecode code = read_u8(vm);
    switch (code) {
    case Bytecode_PushF64:
      push(vm, new_number_value(read_f64(vm)));
      break;
    case Bytecode_Pop:
      break;

    case Bytecode_Negate: {
      double operand = value_as_number(pop(vm));
      push(vm, new_number_value(-operand));
      break;
    }

    case Bytecode_Add:
      BINARY_OP(+);
      break;
    case Bytecode_Sub:
      BINARY_OP(-);
      break;
    case Bytecode_Mul:
      BINARY_OP(*);
      break;
    case Bytecode_Div:
      BINARY_OP(/);
      break;
    }
  }

  return value_as_number(pop(vm));
}