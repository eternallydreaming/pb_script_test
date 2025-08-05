#include "parser.h"
#include "bytecode.h"
#include "expr.h"
#include "lexer.h"
#include "value.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

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

Chunk compile_script(const char *source) {
  Parser parser = {
      .lexer = new_lexer(source),
      .chunk = new_chunk(),
  };

  Expr *expr = sum(&parser);
  // simplify_expr(expr);
  compile_expr(&parser.chunk, expr);

  return parser.chunk;
}