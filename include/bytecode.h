#pragma once

#include "chunk.h"
#include "expr.h"

typedef enum Bytecode {
  Bytecode_PushF64 = 0,
  Bytecode_Pop,

  Bytecode_Negate,

  Bytecode_Add,
  Bytecode_Sub,
  Bytecode_Mul,
  Bytecode_Div,
} Bytecode;

void compile_expr(Chunk *chunk, const Expr *expr);
void disassemble_chunk(const Chunk *chunk);