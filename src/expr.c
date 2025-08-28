#include "expr.h"
#include "lexer.h"
#include "type_def.h"
#include "value.h"
#include <assert.h>
#include <stdlib.h>

BinaryOp token_to_binary_op(TokenType type) {
  switch (type) {
  case TokenType_Plus:
    return BinaryOp_Add;
  case TokenType_Minus:
    return BinaryOp_Subtract;
  case TokenType_Star:
    return BinaryOp_Multiply;
  case TokenType_Slash:
    return BinaryOp_Divide;

  case TokenType_Equal:
    return BinaryOp_Equal;
  case TokenType_NotEqual:
    return BinaryOp_NotEqual;
  case TokenType_Less:
    return BinaryOp_Less;
  case TokenType_LessEqual:
    return BinaryOp_LessEqual;
  case TokenType_Greater:
    return BinaryOp_Greater;
  case TokenType_GreaterEqual:
    return BinaryOp_GreaterEqual;

  case TokenType_And:
    return BinaryOp_And;
  case TokenType_Or:
    return BinaryOp_Or;
  default:
    assert(0);
  }
}

Expr *new_expr(ExprType type, TypeDef return_type) {
  Expr *expr = malloc(sizeof(*expr));
  assert(expr != NULL);
  expr->type = type;
  expr->return_type = return_type;
  expr->next = NULL;
  return expr;
}

Expr *new_literal_expr(Value value) {
  Expr *expr = new_expr(ExprType_Literal, new_type_def(value.type, false));
  expr->literal = value;
  return expr;
}

Expr *new_get_var_expr(size_t idx, TypeDef type) {
  Expr *expr = new_expr(ExprType_GetVar, type);
  expr->get_var.idx = idx;
  return expr;
}

Expr *new_set_var_expr(size_t idx, Expr *value) {
  Expr *expr = new_expr(ExprType_SetVar, value->return_type);
  expr->set_var.idx = idx;
  expr->set_var.value = value;
  return expr;
}

Expr *new_native_call_expr(size_t idx, TypeDef return_type, Expr *argv_head) {
  Expr *expr = new_expr(ExprType_NativeCall, return_type);
  expr->call.idx = idx;
  expr->call.argv_head = argv_head;
  return expr;
}

Expr *new_unary_expr(UnaryOp op, Expr *operand) {
  Expr *expr = new_expr(ExprType_Unary, operand->return_type);
  expr->unary.op = op;
  expr->unary.operand = operand;
  return expr;
}

Expr *new_binary_expr(BinaryOp op, Expr *lhs, Expr *rhs) {
  Expr *expr = new_expr(ExprType_Binary, lhs->return_type);
  if (op >= BinaryOp_Equal)
    expr->return_type = TypeDef_Boolean;
  expr->binary.op = op;
  expr->binary.lhs = lhs;
  expr->binary.rhs = rhs;
  return expr;
}

void delete_expr(Expr *expr) {
  if (expr->next != NULL)
    delete_expr(expr->next);

  switch (expr->type) {
  case ExprType_Literal:
    release_value(expr->literal);
    break;
  case ExprType_GetVar:
    break;
  case ExprType_SetVar:
    delete_expr(expr->set_var.value);
    break;

  case ExprType_NativeCall:
    if (expr->call.argv_head != NULL)
      delete_expr(expr->call.argv_head);
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
  case ExprType_GetVar:
    break;
  case ExprType_SetVar:
    simplify_expr(expr->set_var.value);
    break;

  case ExprType_NativeCall:
    for (Expr *arg = expr->call.argv_head; arg != NULL; arg = arg->next)
      simplify_expr(arg);
    break;

  case ExprType_Unary: {
    simplify_expr(expr->unary.operand);
    Expr *operand = expr->unary.operand;
    if (operand->type != ExprType_Literal)
      break;

    expr->type = ExprType_Literal;
    switch (expr->unary.op) {
    case UnaryOp_Negate:
      expr->literal = new_number_value(-value_as_number(operand->literal));
      break;
    case UnaryOp_Not:
      expr->literal = new_boolean_value(!value_as_boolean(operand->literal));
      break;
    }

    delete_expr(operand);
    break;
  }
  case ExprType_Binary: {
    simplify_expr(expr->binary.lhs);
    simplify_expr(expr->binary.rhs);
    Expr *lhs_expr = expr->binary.lhs, *rhs_expr = expr->binary.rhs;
    if (lhs_expr->type != ExprType_Literal ||
        rhs_expr->type != ExprType_Literal)
      break;
    Value lhs = lhs_expr->literal, rhs = rhs_expr->literal;

    expr->type = ExprType_Literal;
    switch (expr->binary.op) {
    case BinaryOp_Add:
      if (lhs.type == ValueType_Number) {
        expr->literal =
            new_number_value(value_as_number(lhs) + value_as_number(rhs));
      } else {
        // string concat?
        expr->literal = value_concat(lhs, rhs);
      }
      break;
    case BinaryOp_Subtract:
      expr->literal =
          new_number_value(value_as_number(lhs) - value_as_number(rhs));
      break;
    case BinaryOp_Multiply:
      expr->literal =
          new_number_value(value_as_number(lhs) * value_as_number(rhs));
      break;
    case BinaryOp_Divide:
      expr->literal =
          new_number_value(value_as_number(lhs) / value_as_number(rhs));
      break;

    case BinaryOp_Equal:
      expr->literal = new_boolean_value(value_compare(lhs, rhs));
      break;
    case BinaryOp_NotEqual:
      expr->literal = new_boolean_value(!value_compare(lhs, rhs));
      break;
    case BinaryOp_Less:
      expr->literal =
          new_boolean_value(value_as_number(lhs) < value_as_number(rhs));
      break;
    case BinaryOp_LessEqual:
      expr->literal =
          new_boolean_value(value_as_number(lhs) <= value_as_number(rhs));
      break;
    case BinaryOp_Greater:
      expr->literal =
          new_boolean_value(value_as_number(lhs) > value_as_number(rhs));
      break;
    case BinaryOp_GreaterEqual:
      expr->literal =
          new_boolean_value(value_as_number(lhs) >= value_as_number(rhs));
      break;

    case BinaryOp_And:
      expr->literal =
          new_boolean_value(value_as_boolean(lhs) && value_as_boolean(rhs));
      break;
    case BinaryOp_Or:
      expr->literal =
          new_boolean_value(value_as_boolean(lhs) || value_as_boolean(rhs));
      break;
    }

    delete_expr(rhs_expr);
    delete_expr(lhs_expr);
    break;
  }
  }
}