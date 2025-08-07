#include "vm.h"
#include "bytecode.h"
#include "chunk.h"
#include "value.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

static uint8_t read_u8(Vm *vm) { return read_chunk_u8(vm->chunk, &vm->pc); }
static uint16_t read_u16(Vm *vm) { return read_chunk_u16(vm->chunk, &vm->pc); }
static double read_f64(Vm *vm) { return read_chunk_f64(vm->chunk, &vm->pc); }

static void push(Vm *vm, Value value) {
  assert(vm->sp < 128);
  vm->stack[vm->sp++] = value;
}

static Value peek_at(Vm *vm, size_t idx) {
  assert(idx < vm->sp);
  return vm->stack[idx];
}

static Value peek(Vm *vm) { return peek_at(vm, vm->sp - 1); }

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

Value run_vm(Vm *vm) {
#define BINARY_OP(enum_name, op, result_type)                                  \
  case Bytecode_##enum_name: {                                                 \
    double rhs = value_as_number(pop(vm));                                     \
    double lhs = value_as_number(pop(vm));                                     \
                                                                               \
    push(vm, new_##result_type##_value(lhs op rhs));                           \
    break;                                                                     \
  }

  while (vm->pc < vm->chunk->size) {
    Bytecode instruction = read_u8(vm);
    switch (instruction) {
    case Bytecode_PushNull:
      push(vm, new_null_value());
      break;
    case Bytecode_PushNumber:
      push(vm, new_number_value(read_f64(vm)));
      break;
    case Bytecode_PushTrue:
      push(vm, new_boolean_value(true));
      break;
    case Bytecode_PushFalse:
      push(vm, new_boolean_value(false));
      break;
    case Bytecode_Copy:
      push(vm, copy_value(peek(vm)));
      break;
    case Bytecode_Pop:
      pop(vm);
      break;

    case Bytecode_Load:
      push(vm, peek_at(vm, read_u8(vm)));
      break;
    case Bytecode_Store: {
      uint8_t idx = read_u8(vm);
      assert(idx < vm->sp);
      vm->stack[idx] = peek(vm);
      break;
    }

    case Bytecode_Negate: {
      double operand = value_as_number(pop(vm));
      push(vm, new_number_value(-operand));
      break;
    }
    case Bytecode_Not: {
      bool operand = value_as_boolean(pop(vm));
      push(vm, new_boolean_value(!operand));
      break;
    }

      BINARY_OP(Add, +, number)
      BINARY_OP(Subtract, -, number)
      BINARY_OP(Multiply, *, number)
      BINARY_OP(Divide, /, number)

    case Bytecode_Equal: {
      Value rhs = pop(vm);
      Value lhs = pop(vm);
      push(vm, new_boolean_value(value_compare(lhs, rhs)));
      break;
    }
    case Bytecode_NotEqual: {
      Value rhs = pop(vm);
      Value lhs = pop(vm);
      push(vm, new_boolean_value(!value_compare(lhs, rhs)));
      break;
    }
      BINARY_OP(Less, <, boolean)
      BINARY_OP(LessEqual, <=, boolean)
      BINARY_OP(Greater, >, boolean)
      BINARY_OP(GreaterEqual, >=, boolean)

    case Bytecode_Jump:
      vm->pc += read_u16(vm);
      break;
    case Bytecode_JumpBack:
      vm->pc -= read_u16(vm);
      break;
    case Bytecode_JumpIfFalse: {
      uint16_t offset = read_u16(vm);
      if (!value_as_boolean(pop(vm)))
        vm->pc += offset;
      break;
    }
    case Bytecode_JumpIfTrue: {
      uint16_t offset = read_u16(vm);
      if (value_as_boolean(pop(vm)))
        vm->pc += offset;
      break;
    }
    case Bytecode_JumpIfFalseRetain: {
      uint16_t offset = read_u16(vm);
      if (!value_as_boolean(peek(vm)))
        vm->pc += offset;
      else
        pop(vm);
      break;
    }
    case Bytecode_JumpIfTrueRetain: {
      uint16_t offset = read_u16(vm);
      if (value_as_boolean(peek(vm)))
        vm->pc += offset;
      else
        pop(vm);
      break;
    }
    }
  }

  assert(vm->sp == 1);
  return pop(vm);
}