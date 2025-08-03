#include "parser.h"
#include "expr.h"
#include "lexer.h"
#include "value.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Parser {
  const Token *tokens;
  size_t token_idx;
} Parser;

static Token peek(const Parser *parser) {
  return parser->tokens[parser->token_idx];
}

static bool is_eof(const Parser *parser) {
  return peek(parser).type == TokenType_Eof;
}

static Token advance(Parser *parser) {
  Token token = peek(parser);
  if (token.type != TokenType_Eof)
    parser->token_idx++;
  return token;
}

static bool match(Parser *parser, TokenType expected) {
  Token token = peek(parser);
  if (token.type != expected)
    return false;
  advance(parser);
  return true;
}

static Expr *primary(Parser *parser) {
  Token token = peek(parser);
  switch (token.type) {
  case TokenType_Number:
    advance(parser);
    return new_literal_expr((Value){
        .type = ValueType_Number,
        .number = token.number,
    });
  default:
    assert(0);
    break;
  }
}

static Expr *prefix(Parser *parser) {
  if (match(parser, TokenType_Minus))
    return new_unary_expr(UnaryOp_Negate, primary(parser));
  return primary(parser);
}

#define BINARY_OP_FN(name, next, cond)                                         \
  static Expr *name(Parser *parser) {                                          \
    Expr *expr = next(parser);                                                 \
                                                                               \
    Token token = peek(parser);                                                \
    while (cond) {                                                             \
      advance(parser);                                                         \
      expr =                                                                   \
          new_binary_expr(token_to_binary_op(token.type), expr, next(parser)); \
      token = peek(parser);                                                    \
    }                                                                          \
    return expr;                                                               \
  }

BINARY_OP_FN(factor, prefix,
             token.type == TokenType_Star || token.type == TokenType_Slash)
BINARY_OP_FN(sum, factor,
             token.type == TokenType_Plus || token.type == TokenType_Minus)

void compile_script(const Token *tokens) {
  Parser parser = {
      .tokens = tokens,
      .token_idx = 0,
  };

  Expr *expr = sum(&parser);
  simplify_expr(expr);
  assert(expr->type == ExprType_Literal);
  printf("%f\n", expr->literal.number);
}