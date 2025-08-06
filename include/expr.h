#pragma once

#include "lexer.h"
#include "value.h"

typedef enum ExprType {
  ExprType_Literal = 0,
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

typedef struct Expr {
  ExprType type;
  ValueType value_type;
  union {
    Value literal;
    struct {
      UnaryOp op;
      struct Expr *operand;
    } unary;
    struct {
      BinaryOp op;
      struct Expr *lhs, *rhs;
    } binary;
  };
} Expr;

BinaryOp token_to_binary_op(TokenType type);

Expr *new_expr(ExprType type, ValueType value_type);
Expr *new_literal_expr(Value value);
Expr *new_unary_expr(UnaryOp op, Expr *operand);
Expr *new_binary_expr(BinaryOp op, Expr *lhs, Expr *rhs);
void delete_expr(Expr *expr);

void simplify_expr(Expr *expr);