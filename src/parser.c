#include "parser.h"
#include "bytecode.h"
#include "chunk.h"
#include "expr.h"
#include "lexer.h"
#include "value.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { MAX_VARS = 128 };

typedef enum Symbol {
  Symbol_None = 0,
  Symbol_Var,
} Symbol;

typedef struct Var {
  char *name;
  ValueType type;
  size_t block_depth;
} Var;

typedef struct Parser {
  Lexer lexer;

  size_t block_level;
  Var vars[MAX_VARS];
  size_t vars_num;

  Chunk chunk;
} Parser;

static Token peek(const Parser *parser) { return lexer_peek(&parser->lexer); }

static bool is_eof(const Parser *parser) {
  return peek(parser).type == TokenType_Eof;
}

static void set_context(Parser *parser, LexerContext context) {
  parser->lexer.context = context;
}

static Token advance(Parser *parser) { return lexer_advance(&parser->lexer); }

static bool match(Parser *parser, TokenType what) {
  Token token = peek(parser);
  if (token.type != what)
    return false;
  advance(parser);
  return true;
}

static Token expect(Parser *parser, TokenType what, const char *error_msg) {
  Token token = peek(parser);
  if (token.type != what) {
    puts(error_msg);
    exit(-1);
  }
  advance(parser);
  return token;
}

static Symbol lookup_symbol(const Parser *parser, const char *name,
                            size_t name_len, size_t *out_idx) {
  for (size_t i = parser->vars_num - 1; i < MAX_VARS; i--) {
    if (strncmp(name, parser->vars[i].name, name_len) == 0) {
      if (out_idx != NULL)
        *out_idx = i;
      return Symbol_Var;
    }
  }
  return Symbol_None;
}

static size_t new_var(Parser *parser, const char *name, size_t name_len,
                      ValueType type) {
  if (parser->vars_num >= MAX_VARS) {
    puts("too many variables");
    exit(-1);
  }
  if (lookup_symbol(parser, name, name_len, NULL)) {
    puts("symbol already defined");
    exit(-1);
  }

  parser->vars[parser->vars_num++] = (Var){
      .name = strndup(name, name_len),
      .type = type,
      .block_depth = parser->block_level,
  };
  return parser->vars_num - 1;
}

static void pop_var(Parser *parser) {
  assert(parser->vars_num != 0);

  free(parser->vars[parser->vars_num - 1].name);
  parser->vars_num--;
  write_chunk_u8(&parser->chunk, Bytecode_Pop);
}

static Expr *expr_base(Parser *parser);

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
  case TokenType_Identifier: {
    advance(parser);
    size_t sym_idx;
    Symbol sym =
        lookup_symbol(parser, token.text.start, token.text.len, &sym_idx);
    if (sym == Symbol_None) {
      puts("symbol not defined");
      exit(-1);
    }

    return new_get_var_expr(sym_idx, parser->vars[sym_idx].type);
  }

  case TokenType_LParen: {
    advance(parser);
    Expr *expr = expr_base(parser);
    expect(parser, TokenType_RParen, "expected ')' to close '('");
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
                                                                               \
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

static Expr *expr_base(Parser *parser) {
  Expr *expr = logic_or(parser);
  if (expr->type == ExprType_GetVar && match(parser, TokenType_Assign)) {
    Expr *value = expr_base(parser);
    if (value->value_type != expr->value_type) {
      puts("differing types");
      exit(-1);
    }

    expr->type = ExprType_SetVar;
    expr->set_var.idx = expr->get_var.idx;
    expr->set_var.value = value;
  }

  return expr;
}

static void finalize_expr(Parser *parser, Expr *expr) {
  // simplify_expr(expr);
  compile_expr(&parser->chunk, expr);
  delete_expr(expr);
}

static void statement(Parser *parser);

static void block(Parser *parser) {
  parser->block_level++;
  while (!is_eof(parser) && peek(parser).type != TokenType_RBrace)
    statement(parser);
  expect(parser, TokenType_RBrace, "expected '}' to close '{'");

  // delete variables from this scope
  while (parser->vars_num != 0) {
    if (parser->vars[parser->vars_num - 1].block_depth != parser->block_level)
      break;

    pop_var(parser);
  }
  parser->block_level--;
}

static void if_statement(Parser *parser) {
  Expr *cond = expr_base(parser);
  if (cond->value_type != ValueType_Boolean) {
    puts("if condition must be a boolean");
    exit(-1);
  }
  finalize_expr(parser, cond);

  // skip over the true body if the condition is false
  write_chunk_u8(&parser->chunk, Bytecode_JumpIfFalse);
  size_t skip_true_hole = write_chunk_hole(&parser->chunk, 16);

  expect(parser, TokenType_LBrace, "expected '{' after if condition");
  block(parser);

  if (match(parser, TokenType_Else)) {
    // make sure to skip over the false body if the condition is true
    write_chunk_u8(&parser->chunk, Bytecode_Jump);
    size_t skip_false_hole = write_chunk_hole(&parser->chunk, 16);
    patch_chunk_hole_u16(&parser->chunk, skip_true_hole);

    expect(parser, TokenType_LBrace, "expected '{' after 'else'");
    block(parser);

    patch_chunk_hole_u16(&parser->chunk, skip_false_hole);
  } else {
    patch_chunk_hole_u16(&parser->chunk, skip_true_hole);
  }
}

static void while_statement(Parser *parser) {
  Expr *cond = expr_base(parser);
  if (cond->value_type != ValueType_Boolean) {
    puts("while condition must be a boolean");
    exit(-1);
  }

  size_t cond_pos = parser->chunk.size;
  finalize_expr(parser, cond);
  // skip over the body if the condition is false
  write_chunk_u8(&parser->chunk, Bytecode_JumpIfFalse);
  size_t skip_body_hole = write_chunk_hole(&parser->chunk, 16);

  expect(parser, TokenType_LBrace, "expected '{' after while condition");
  block(parser);
  // back to the condition
  write_chunk_u8(&parser->chunk, Bytecode_JumpBack);
  write_chunk_u16(&parser->chunk,
                  parser->chunk.size - cond_pos + sizeof(uint16_t));

  patch_chunk_hole_u16(&parser->chunk, skip_body_hole);
}

static void for_statement(Parser *parser) {
  set_context(parser, LexerContext_For);

  Token counter_token =
      expect(parser, TokenType_Identifier, "expected identifier after 'for'");
  expect(parser, TokenType_In, "expected 'in' after loop counter");

  Expr *from = expr_base(parser);
  if (from->value_type != ValueType_Number) {
    puts("from expression must be a number");
    exit(-1);
  }
  simplify_expr(from);

  bool inclusive = false;
  if (match(parser, TokenType_FatArrow)) {
    inclusive = true;
  } else if (!match(parser, TokenType_Arrow)) {
    puts("expected '->' or '=>' after from expression");
    exit(-1);
  }

  Expr *to = expr_base(parser);
  if (to->value_type != ValueType_Number) {
    puts("to expression must be a number");
    exit(-1);
  }
  simplify_expr(to);

  double step = 1.0;
  if (match(parser, TokenType_By)) {
    Expr *step_expr = expr_base(parser);
    if (step_expr->value_type != ValueType_Number) {
      puts("step expression must be a number");
      exit(-1);
    }

    simplify_expr(step_expr);
    if (step_expr->type != ExprType_Literal) {
      puts("step expression must be a constant");
      exit(-1);
    }

    step = value_as_number(step_expr->literal);
    delete_expr(step_expr);
  } else if (from->type == ExprType_Literal && to->type == ExprType_Literal) {
    // const range, check if we're going backwards
    if (value_as_number(from->literal) > value_as_number(to->literal))
      step = -1.0;
  }

  set_context(parser, LexerContext_None);

  size_t counter_idx = new_var(parser, counter_token.text.start,
                               counter_token.text.len, ValueType_Number);

  // init
  finalize_expr(parser, from);

  // condition
  size_t cond_pos = parser->chunk.size;
  write_chunk_u8(&parser->chunk, Bytecode_Copy);
  finalize_expr(parser, to);

  if (step >= 0.0) {
    write_chunk_u8(&parser->chunk,
                   (inclusive) ? Bytecode_LessEqual : Bytecode_Less);
  } else {
    write_chunk_u8(&parser->chunk,
                   (inclusive) ? Bytecode_GreaterEqual : Bytecode_Greater);
  }

  write_chunk_u8(&parser->chunk, Bytecode_JumpIfFalse);
  size_t skip_body_hole = write_chunk_hole(&parser->chunk, 16);

  // body
  expect(parser, TokenType_LBrace, "expected '{' after to expression");
  block(parser);

  // increment
  write_chunk_u8(&parser->chunk, Bytecode_Load);
  write_chunk_u8(&parser->chunk, counter_idx);
  write_chunk_u8(&parser->chunk, Bytecode_PushNumber);
  write_chunk_f64(&parser->chunk, step);
  write_chunk_u8(&parser->chunk, Bytecode_Add);
  write_chunk_u8(&parser->chunk, Bytecode_Store);
  write_chunk_u8(&parser->chunk, counter_idx);
  write_chunk_u8(&parser->chunk, Bytecode_Pop);
  // back to the condition
  write_chunk_u8(&parser->chunk, Bytecode_JumpBack);
  write_chunk_u16(&parser->chunk,
                  parser->chunk.size - cond_pos + sizeof(uint16_t));

  patch_chunk_hole_u16(&parser->chunk, skip_body_hole);

  // ..and make sure to get rid of the counter variable
  pop_var(parser);
}

static void var_decl(Parser *parser) {
  Token name_token =
      expect(parser, TokenType_Identifier, "expected identifier after 'let'");
  expect(parser, TokenType_Assign, "expected '=' after identifier");

  Expr *value = expr_base(parser);
  new_var(parser, name_token.text.start, name_token.text.len,
          value->value_type);
  finalize_expr(parser, value);

  expect(parser, TokenType_Semicolon,
         "expected ';' after variable declaration");
}

static void statement(Parser *parser) {
  if (match(parser, TokenType_If)) {
    if_statement(parser);
  } else if (match(parser, TokenType_While)) {
    while_statement(parser);
  } else if (match(parser, TokenType_For)) {
    for_statement(parser);
  } else if (match(parser, TokenType_Let)) {
    var_decl(parser);
  } else if (match(parser, TokenType_LBrace)) {
    block(parser);
  } else {
    // expression
    finalize_expr(parser, expr_base(parser));
    expect(parser, TokenType_Semicolon, "expected ';' after expression");

    write_chunk_u8(&parser->chunk, Bytecode_Pop);
  }
}

Chunk compile_script(const char *source) {
  Parser parser = {
      .lexer = new_lexer(source),

      .block_level = 0,
      .vars = {},
      .vars_num = 0,

      .chunk = new_chunk(),
  };

  while (!is_eof(&parser))
    statement(&parser);
  return parser.chunk;
}