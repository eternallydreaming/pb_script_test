#include "lexer.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static Token emit(Lexer *lexer, TokenType type) {
  lexer->token = (Token){
      .type = type,
  };
  return lexer->token;
}

static Token emit_number(Lexer *lexer, TokenType type, double number) {
  lexer->token = (Token){
      .type = type,
      .number = number,
  };
  return lexer->token;
}

static char peek(const Lexer *lexer) {
  if (lexer->pos >= lexer->source_len)
    return 0;
  return lexer->source[lexer->pos];
}

static char advance(Lexer *lexer) {
  char ch = peek(lexer);
  if (ch == 0)
    return ch;
  lexer->pos++;
  return ch;
}

static Token read_number(Lexer *lexer) {
  const char *start = lexer->source + lexer->pos;
  while (isdigit(peek(lexer)))
    advance(lexer);

  char delim = peek(lexer);
  if (delim == '.' || delim == 'x' || delim == 'X') {
    advance(lexer);
    while (isdigit(peek(lexer)))
      advance(lexer);
  }

  return emit_number(lexer, TokenType_Number, atof(start));
}

Lexer new_lexer(const char *source) {
  Lexer lexer = {
      .source = source,
      .source_len = strlen(source),
      .pos = 0,

      .token = {},
  };
  lexer_advance(&lexer);

  return lexer;
}

Token lexer_peek(const Lexer *lexer) { return lexer->token; }

Token lexer_advance(Lexer *lexer) {
  while (isspace(peek(lexer)))
    advance(lexer);

  char ch = peek(lexer);
  switch (ch) {
  case 0:
    return emit(lexer, TokenType_Eof);
  case '+':
    advance(lexer);
    return emit(lexer, TokenType_Plus);
  case '-':
    advance(lexer);
    return emit(lexer, TokenType_Minus);
  case '*':
    advance(lexer);
    return emit(lexer, TokenType_Star);
  case '/':
    advance(lexer);
    return emit(lexer, TokenType_Slash);
  default:
    if (isdigit(ch))
      return read_number(lexer);
    advance(lexer);
    return emit(lexer, TokenType_Error);
  }
}