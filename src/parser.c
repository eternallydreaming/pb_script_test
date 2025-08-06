#include "parser.h"
#include "bytecode.h"
#include "expr.h"
#include "lexer.h"
#include "value.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Parser {
  Lexer lexer;
  Chunk chunk;
} Parser;

static Token peek(const Parser *parser) { return lexer_peek(&parser->lexer); }

static bool is_eof(const Parser *parser) {
  return peek(parser).type == TokenType_Eof;
}

static Token advance(Parser *parser) { return lexer_advance(&parser->lexer); }

static bool match(Parser *parser, TokenType expected) {
  Token token = peek(parser);
  if (token.type != expected)
    return false;
  advance(parser);
  return true;
}

static Expr *logic_or(Parser *parser);

static Expr *primary(Parser *parser) {
  Token token = peek(parser);
  switch (token.type) {
  case TokenType_Null:
    advance(parser);
    return new_literal_expr(new_null_value());
  case TokenType_True:
    advance(parser);
    return new_literal_expr(new_boolean_value(true));
  case TokenType_False:
    advance(parser);
    return new_literal_expr(new_boolean_value(false));

  case TokenType_Number:
    advance(parser);
    return new_literal_expr(new_number_value(token.number));

  case TokenType_LParen: {
    advance(parser);

    Expr *expr = logic_or(parser);
    if (!match(parser, TokenType_RParen)) {
      puts("unclosed parenthesis");
      exit(-1);
    }
    return expr;
  }
  default:
    assert(0);
    break;
  }
}

static Expr *prefix(Parser *parser) {
  if (match(parser, TokenType_Minus)) {
    Expr *operand = primary(parser);
    if (operand->value_type != ValueType_Number) {
      puts("invalid negate operation");
      exit(-1);
    }

    return new_unary_expr(UnaryOp_Negate, operand);
  } else if (match(parser, TokenType_Bang)) {
    Expr *operand = primary(parser);
    if (operand->value_type != ValueType_Boolean) {
      puts("invalid logic not operation");
      exit(-1);
    }

    return new_unary_expr(UnaryOp_Not, operand);
  }

  return primary(parser);
}

static bool check_numbers(Expr *lhs, Expr *rhs) {
  return lhs->value_type == ValueType_Number &&
         rhs->value_type == ValueType_Number;
}

static bool check_equality(Expr *lhs, Expr *rhs) {
  return lhs->value_type == rhs->value_type ||
         lhs->value_type == ValueType_Null || rhs->value_type == ValueType_Null;
}

static bool check_booleans(Expr *lhs, Expr *rhs) {
  return lhs->value_type == ValueType_Boolean &&
         rhs->value_type == ValueType_Boolean;
}

#define BINARY_OP_FN(name, next_fn, check_fn, cond)                            \
  static Expr *name(Parser *parser) {                                          \
    Expr *lhs = next_fn(parser);                                               \
                                                                               \
    Token token = peek(parser);                                                \
    while (cond) {                                                             \
      advance(parser);                                                         \
      Expr *rhs = next_fn(parser);                                             \
      if (!check_fn(lhs, rhs)) {                                               \
        puts("invalid binary operation");                                      \
        exit(-1);                                                              \
      }                                                                        \
                                                                               \
      lhs = new_binary_expr(token_to_binary_op(token.type), lhs, rhs);         \
      token = peek(parser);                                                    \
    }                                                                          \
    return lhs;                                                                \
  }

BINARY_OP_FN(factor, prefix, check_numbers,
             token.type == TokenType_Star || token.type == TokenType_Slash)
BINARY_OP_FN(sum, factor, check_numbers,
             token.type == TokenType_Plus || token.type == TokenType_Minus)
BINARY_OP_FN(rel_test, sum, check_numbers,
             token.type == TokenType_Less ||
                 token.type == TokenType_LessEqual ||
                 token.type == TokenType_Greater ||
                 token.type == TokenType_GreaterEqual)
BINARY_OP_FN(test, rel_test, check_equality,
             token.type == TokenType_Equal || token.type == TokenType_NotEqual)
BINARY_OP_FN(logic_and, test, check_booleans, token.type == TokenType_And)
BINARY_OP_FN(logic_or, logic_and, check_booleans, token.type == TokenType_Or)

Chunk compile_script(const char *source) {
  Parser parser = {
      .lexer = new_lexer(source),
      .chunk = new_chunk(),
  };

  Expr *expr = logic_or(&parser);
  // simplify_expr(expr);
  compile_expr(&parser.chunk, expr);

  return parser.chunk;
}