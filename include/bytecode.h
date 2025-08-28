#pragma once

#include "chunk.h"
#include "expr.h"
#include "registry.h"

typedef enum Bytecode {
  Bytecode_PushNull = 0,
  Bytecode_PushNumber,
  Bytecode_PushTrue,
  Bytecode_PushFalse,
  Bytecode_PushString,
  Bytecode_Copy,
  Bytecode_Pop,

  Bytecode_Load,
  Bytecode_Store,

  Bytecode_NativeCall,

  Bytecode_Negate,
  Bytecode_Not,

  Bytecode_Add,
  Bytecode_Subtract,
  Bytecode_Multiply,
  Bytecode_Divide,

  Bytecode_Equal,
  Bytecode_NotEqual,
  Bytecode_Less,
  Bytecode_LessEqual,
  Bytecode_Greater,
  Bytecode_GreaterEqual,

  Bytecode_Concat,

  Bytecode_Jump,
  Bytecode_JumpBack,
  Bytecode_JumpIfFalse,
  Bytecode_JumpIfTrue,
  Bytecode_JumpIfFalseRetain,
  Bytecode_JumpIfTrueRetain,
} Bytecode;

void compile_expr(Chunk *chunk, const Expr *expr);
void disassemble_chunk(const Chunk *chunk, const Registry *registry);