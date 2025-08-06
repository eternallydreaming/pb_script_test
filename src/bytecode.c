#include "bytecode.h"
#include "chunk.h"
#include "expr.h"
#include "value.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void compile_expr(Chunk *chunk, const Expr *expr) {
  switch (expr->type) {
  case ExprType_Literal:
    switch (expr->literal.type) {
    case ValueType_Null:
      write_chunk_u8(chunk, Bytecode_PushNull);
      break;
    case ValueType_Number:
      write_chunk_u8(chunk, Bytecode_PushNumber);
      write_chunk_f64(chunk, expr->literal.number);
      break;
    case ValueType_Boolean:
      write_chunk_u8(chunk, (expr->literal.boolean) ? Bytecode_PushTrue
                                                    : Bytecode_PushFalse);
      break;
    }
    break;
  case ExprType_Unary:
    compile_expr(chunk, expr->unary.operand);

    switch (expr->unary.op) {
    case UnaryOp_Negate:
      write_chunk_u8(chunk, Bytecode_Negate);
      break;
    case UnaryOp_Not:
      write_chunk_u8(chunk, Bytecode_Not);
      break;
    }
    break;
  case ExprType_Binary:
    compile_expr(chunk, expr->binary.lhs);
    // and & or are short-circuiting, so don't write the right hand side yet if
    // that's what we're compiling
    if (expr->binary.op != BinaryOp_And && expr->binary.op != BinaryOp_Or)
      compile_expr(chunk, expr->binary.rhs);

    switch (expr->binary.op) {
    case BinaryOp_Add:
      write_chunk_u8(chunk, Bytecode_Add);
      break;
    case BinaryOp_Subtract:
      write_chunk_u8(chunk, Bytecode_Subtract);
      break;
    case BinaryOp_Multiply:
      write_chunk_u8(chunk, Bytecode_Multiply);
      break;
    case BinaryOp_Divide:
      write_chunk_u8(chunk, Bytecode_Divide);
      break;

    case BinaryOp_Equal:
      write_chunk_u8(chunk, Bytecode_Equal);
      break;
    case BinaryOp_NotEqual:
      write_chunk_u8(chunk, Bytecode_NotEqual);
      break;
    case BinaryOp_Less:
      write_chunk_u8(chunk, Bytecode_Less);
      break;
    case BinaryOp_LessEqual:
      write_chunk_u8(chunk, Bytecode_LessEqual);
      break;
    case BinaryOp_Greater:
      write_chunk_u8(chunk, Bytecode_Greater);
      break;
    case BinaryOp_GreaterEqual:
      write_chunk_u8(chunk, Bytecode_GreaterEqual);
      break;

    case BinaryOp_And: {
      write_chunk_u8(chunk, Bytecode_JumpIfFalseRetain);
      size_t hole_pos = write_chunk_hole(chunk, 16);

      // rhs is only evaluated if lhs is true
      compile_expr(chunk, expr->binary.rhs);
      patch_chunk_hole_u16(chunk, hole_pos);
      break;
    }
    case BinaryOp_Or: {
      write_chunk_u8(chunk, Bytecode_JumpIfTrueRetain);
      size_t hole_pos = write_chunk_hole(chunk, 16);

      // rhs is only evaluated if lhs is false
      compile_expr(chunk, expr->binary.rhs);
      patch_chunk_hole_u16(chunk, hole_pos);
      break;
    }
    }
    break;
  }
}

void disassemble_chunk(const Chunk *chunk) {
  size_t pos = 0;
  while (pos < chunk->size) {
    printf("%zu\t| ", pos);

    Bytecode instruction = read_chunk_u8(chunk, &pos);
    switch (instruction) {
    case Bytecode_PushNull:
      printf("push_null\n");
      break;
    case Bytecode_PushNumber:
      printf("push_number %f\n", read_chunk_f64(chunk, &pos));
      break;
    case Bytecode_PushTrue:
      printf("push_true\n");
      break;
    case Bytecode_PushFalse:
      printf("push_false\n");
      break;
    case Bytecode_Copy:
      printf("copy\n");
      break;
    case Bytecode_Pop:
      printf("pop\n");
      break;

    case Bytecode_Negate:
      printf("negate\n");
      break;
    case Bytecode_Not:
      printf("not\n");
      break;

    case Bytecode_Add:
      printf("add\n");
      break;
    case Bytecode_Subtract:
      printf("subtract\n");
      break;
    case Bytecode_Multiply:
      printf("multiply\n");
      break;
    case Bytecode_Divide:
      printf("divide\n");
      break;

    case Bytecode_Equal:
      printf("equal\n");
      break;
    case Bytecode_NotEqual:
      printf("not_equal\n");
      break;
    case Bytecode_Less:
      printf("less\n");
      break;
    case Bytecode_LessEqual:
      printf("less_equal\n");
      break;
    case Bytecode_Greater:
      printf("greater\n");
      break;
    case Bytecode_GreaterEqual:
      printf("greater_equal\n");
      break;

    case Bytecode_JumpIfFalse:
      printf("jump_if_false +%d\n", read_chunk_u16(chunk, &pos));
      break;
    case Bytecode_JumpIfTrue:
      printf("jump_if_true +%d\n", read_chunk_u16(chunk, &pos));
      break;
    case Bytecode_JumpIfFalseRetain:
      printf("jump_if_false_retain +%d\n", read_chunk_u16(chunk, &pos));
      break;
    case Bytecode_JumpIfTrueRetain:
      printf("jump_if_true_retain +%d\n", read_chunk_u16(chunk, &pos));
      break;
    }
  }
}