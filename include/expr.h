#pragma once

#include "lexer.h"
#include "type_def.h"
#include "value.h"

typedef enum ExprType {
  ExprType_Literal = 0,
  ExprType_GetVar,
  ExprType_SetVar,

  ExprType_NativeCall,

  ExprType_Unary,
  ExprType_Binary,
} ExprType;

typedef enum UnaryOp {
  UnaryOp_Negate = 0,
  UnaryOp_Not,
} UnaryOp;

typedef enum BinaryOp {
  BinaryOp_Add = 0,
  BinaryOp_Subtract,
  BinaryOp_Multiply,
  BinaryOp_Divide,

  BinaryOp_Equal, // anything below this will be treated as having a boolean
                  // result
  BinaryOp_NotEqual,
  BinaryOp_Less,
  BinaryOp_LessEqual,
  BinaryOp_Greater,
  BinaryOp_GreaterEqual,

  BinaryOp_And,
  BinaryOp_Or,
} BinaryOp;

struct ExprList;
typedef struct Expr {
  ExprType type;
  TypeDef return_type;
  union {
    Value literal;
    struct {
      size_t idx;
    } get_var;
    struct {
      size_t idx;
      struct Expr *value;
    } set_var;

    struct {
      size_t idx;
      struct Expr *argv_head;
    } call;

    struct {
      UnaryOp op;
      struct Expr *operand;
    } unary;
    struct {
      BinaryOp op;
      struct Expr *lhs, *rhs;
    } binary;
  };
  struct Expr *next;
} Expr;

BinaryOp token_to_binary_op(TokenType type);

Expr *new_expr(ExprType type, TypeDef return_type);
Expr *new_literal_expr(Value value);
Expr *new_get_var_expr(size_t idx, TypeDef type);
Expr *new_set_var_expr(size_t idx, Expr *value);
Expr *new_native_call_expr(size_t idx, TypeDef return_type, Expr *argv_head);
Expr *new_unary_expr(UnaryOp op, Expr *operand);
Expr *new_binary_expr(BinaryOp op, Expr *lhs, Expr *rhs);
void delete_expr(Expr *expr);

void simplify_expr(Expr *expr);