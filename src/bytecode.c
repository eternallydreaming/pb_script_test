#include "bytecode.h"
#include "chunk.h"
#include "expr.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void compile_expr(Chunk *chunk, const Expr *expr) {
  switch (expr->type) {
  case ExprType_Literal:
    write_chunk_u8(chunk, Bytecode_PushF64);
    write_chunk_f64(chunk, expr->literal.number);
    break;
  case ExprType_Unary:
    compile_expr(chunk, expr->unary.operand);

    switch (expr->unary.op) {
    case UnaryOp_Group:
      break;
    case UnaryOp_Negate:
      write_chunk_u8(chunk, Bytecode_Negate);
      break;
    }
    break;
  case ExprType_Binary:
    compile_expr(chunk, expr->binary.lhs);
    compile_expr(chunk, expr->binary.rhs);

    switch (expr->binary.op) {
    case BinaryOp_Add:
      write_chunk_u8(chunk, Bytecode_Add);
      break;
    case BinaryOp_Sub:
      write_chunk_u8(chunk, Bytecode_Sub);
      break;
    case BinaryOp_Mul:
      write_chunk_u8(chunk, Bytecode_Mul);
      break;
    case BinaryOp_Div:
      write_chunk_u8(chunk, Bytecode_Div);
      break;
    }
    break;
  }
}

void disassemble_chunk(const Chunk *chunk) {
  size_t pos = 0;
  while (pos < chunk->size) {
    Bytecode instruction = read_chunk_u8(chunk, &pos);
    switch (instruction) {
    case Bytecode_PushF64:
      printf("push_f64 %f\n", read_chunk_f64(chunk, &pos));
      break;
    case Bytecode_Pop:
      printf("pop\n");
      break;

    case Bytecode_Negate:
      printf("negate\n");
      break;

    case Bytecode_Add:
      printf("add\n");
      break;
    case Bytecode_Sub:
      printf("sub\n");
      break;
    case Bytecode_Mul:
      printf("mul\n");
      break;
    case Bytecode_Div:
      printf("div\n");
      break;
    }
  }
}