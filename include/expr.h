#pragma once

#include "lexer.h"
#include "value.h"

typedef enum ExprType {
  ExprType_Literal = 0,
  ExprType_Unary,
  ExprType_Binary,
} ExprType;

typedef enum UnaryOp {
  UnaryOp_Group = 0,
  UnaryOp_Negate,
} UnaryOp;

typedef enum BinaryOp {
  BinaryOp_Add = 0,
  BinaryOp_Sub,
  BinaryOp_Mul,
  BinaryOp_Div,
} BinaryOp;

typedef struct Expr {
  ExprType type;
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

Expr *new_expr(ExprType type);
Expr *new_literal_expr(Value value);
Expr *new_unary_expr(UnaryOp op, Expr *operand);
Expr *new_binary_expr(BinaryOp op, Expr *lhs, Expr *rhs);
void delete_expr(Expr *expr);

void simplify_expr(Expr *expr);