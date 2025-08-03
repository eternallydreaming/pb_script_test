#include "expr.h"
#include "lexer.h"
#include "value.h"
#include <assert.h>
#include <stdlib.h>

BinaryOp token_to_binary_op(TokenType type) {
  switch (type) {
  case TokenType_Plus:
    return BinaryOp_Add;
  case TokenType_Minus:
    return BinaryOp_Sub;
  case TokenType_Star:
    return BinaryOp_Mul;
  case TokenType_Slash:
    return BinaryOp_Div;
  default:
    assert(0);
  }
}

Expr *new_expr(ExprType type) {
  Expr *expr = malloc(sizeof(*expr));
  assert(expr != NULL);
  expr->type = type;
  return expr;
}

Expr *new_literal_expr(Value value) {
  Expr *expr = new_expr(ExprType_Literal);
  expr->literal = value;
  return expr;
}

Expr *new_unary_expr(UnaryOp op, Expr *operand) {
  Expr *expr = new_expr(ExprType_Unary);
  expr->unary.op = op;
  expr->unary.operand = operand;
  return expr;
}

Expr *new_binary_expr(BinaryOp op, Expr *lhs, Expr *rhs) {
  Expr *expr = new_expr(ExprType_Binary);
  expr->binary.op = op;
  expr->binary.lhs = lhs;
  expr->binary.rhs = rhs;
  return expr;
}

void delete_expr(Expr *expr) {
  switch (expr->type) {
  case ExprType_Literal:
    break;
  case ExprType_Unary:
    delete_expr(expr->unary.operand);
    break;
  case ExprType_Binary:
    delete_expr(expr->binary.rhs);
    delete_expr(expr->binary.lhs);
    break;
  }
  free(expr);
}

void simplify_expr(Expr *expr) {
  switch (expr->type) {
  case ExprType_Literal:
    break;
  case ExprType_Unary: {
    simplify_expr(expr->unary.operand);
    Expr *operand = expr->unary.operand;
    if (operand->type != ExprType_Literal)
      break;

    Value result = {
        .type = ValueType_Number,
    };
    switch (expr->unary.op) {
    case UnaryOp_Negate:
      result.number = -operand->literal.number;
      break;
    case UnaryOp_Group:
      result.number = operand->literal.number;
      break;
    }

    delete_expr(expr);
    expr = new_literal_expr(result);
    break;
  }
  case ExprType_Binary: {
    simplify_expr(expr->binary.lhs);
    simplify_expr(expr->binary.rhs);
    Expr *lhs = expr->binary.lhs, *rhs = expr->binary.rhs;
    if (lhs->type != ExprType_Literal || rhs->type != ExprType_Literal)
      break;

    Value result = {
        .type = ValueType_Number,
    };
    switch (expr->binary.op) {
    case BinaryOp_Add:
      result.number = lhs->literal.number + rhs->literal.number;
      break;
    case BinaryOp_Sub:
      result.number = lhs->literal.number - rhs->literal.number;
      break;
    case BinaryOp_Mul:
      result.number = lhs->literal.number * rhs->literal.number;
      break;
    case BinaryOp_Div:
      result.number = lhs->literal.number / rhs->literal.number;
      break;
    }

    delete_expr(expr);
    expr = new_literal_expr(result);
    break;
  }
  }
}