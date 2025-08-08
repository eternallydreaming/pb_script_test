#include "bytecode.h"
#include "chunk.h"
#include "expr.h"
#include "value.h"
#include <assert.h>
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
    case ValueType_String: {
      size_t idx = add_chunk_string(chunk, expr->literal.string->chars,
                                    expr->literal.string->len);
      write_chunk_u8(chunk, Bytecode_PushString);
      write_chunk_u16(chunk, idx);
      break;
    }
    }
    break;
  case ExprType_GetVar:
    write_chunk_u8(chunk, Bytecode_Load);
    write_chunk_u8(chunk, expr->get_var.idx);
    break;
  case ExprType_SetVar:
    compile_expr(chunk, expr->set_var.value);

    write_chunk_u8(chunk, Bytecode_Store);
    write_chunk_u8(chunk, expr->set_var.idx);
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
      // special bytecode for adding strings
      if (expr->binary.lhs->value_type == ValueType_String)
        write_chunk_u8(chunk, Bytecode_Concat);
      else
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
      size_t skip_rhs_hole = write_chunk_hole(chunk, 16);

      // rhs is only evaluated if lhs is true
      compile_expr(chunk, expr->binary.rhs);
      patch_chunk_hole_u16(chunk, skip_rhs_hole);
      break;
    }
    case BinaryOp_Or: {
      write_chunk_u8(chunk, Bytecode_JumpIfTrueRetain);
      size_t skip_rhs_hole = write_chunk_hole(chunk, 16);

      // rhs is only evaluated if lhs is false
      compile_expr(chunk, expr->binary.rhs);
      patch_chunk_hole_u16(chunk, skip_rhs_hole);
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
    case Bytecode_PushString: {
      uint16_t idx = read_chunk_u16(chunk, &pos);
      assert(idx < chunk->strings_num);
      printf("push_string \"%s\"\n", chunk->strings[idx].chars);
      break;
    }
    case Bytecode_Copy:
      printf("copy\n");
      break;
    case Bytecode_Pop:
      printf("pop\n");
      break;

    case Bytecode_Load:
      printf("load $%d\n", read_chunk_u8(chunk, &pos));
      break;
    case Bytecode_Store:
      printf("store $%d\n", read_chunk_u8(chunk, &pos));
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

    case Bytecode_Concat:
      printf("concat\n");
      break;

    case Bytecode_Jump:
      printf("jump +%d\n", read_chunk_u16(chunk, &pos));
      break;
    case Bytecode_JumpBack:
      printf("jump_back -%d\n", read_chunk_u16(chunk, &pos));
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