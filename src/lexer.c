#include "lexer.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static char peek(const Lexer *lexer) { return lexer->source[lexer->pos]; }

static char advance(Lexer *lexer) {
  char ch = peek(lexer);
  if (ch == 0)
    return ch;
  lexer->pos++;
  return ch;
}

static bool match(Lexer *lexer, char expected) {
  char ch = peek(lexer);
  if (ch != expected)
    return false;
  advance(lexer);
  return true;
}

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

static Token read_number(Lexer *lexer) {
  size_t start_pos = lexer->pos - 1;
  while (isdigit(peek(lexer)))
    advance(lexer);

  char delim = peek(lexer);
  if (delim == '.' || delim == 'x' || delim == 'X') {
    advance(lexer);
    while (isdigit(peek(lexer)))
      advance(lexer);
  }

  return emit_number(lexer, TokenType_Number, atof(lexer->source + start_pos));
}

static Token read_identifier(Lexer *lexer) {
  size_t start_pos = lexer->pos - 1;
  while (isalnum(peek(lexer)) || peek(lexer) == '_')
    advance(lexer);
  size_t len = lexer->pos - start_pos;

  const char *ident = lexer->source + start_pos;
  if (strncmp(ident, "null", len) == 0)
    return emit(lexer, TokenType_Null);
  else if (strncmp(ident, "true", len) == 0)
    return emit(lexer, TokenType_True);
  else if (strncmp(ident, "false", len) == 0)
    return emit(lexer, TokenType_False);
  abort();
}

Lexer new_lexer(const char *source) {
  Lexer lexer = {
      .source = source,
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

  char ch = advance(lexer);
  switch (ch) {
  case 0:
    return emit(lexer, TokenType_Eof);

  case '(':
    return emit(lexer, TokenType_LParen);
  case ')':
    return emit(lexer, TokenType_RParen);

  case '+':
    return emit(lexer, TokenType_Plus);
  case '-':
    return emit(lexer, TokenType_Minus);
  case '*':
    return emit(lexer, TokenType_Star);
  case '/':
    return emit(lexer, TokenType_Slash);

  case '!':
    if (match(lexer, '='))
      return emit(lexer, TokenType_NotEqual);
    return emit(lexer, TokenType_Bang);
  case '=':
    if (match(lexer, '='))
      return emit(lexer, TokenType_Equal);
    exit(-1);
    break;
  case '<':
    if (match(lexer, '='))
      return emit(lexer, TokenType_LessEqual);
    return emit(lexer, TokenType_Less);
  case '>':
    if (match(lexer, '='))
      return emit(lexer, TokenType_GreaterEqual);
    return emit(lexer, TokenType_Greater);

  case '&':
    if (match(lexer, '&'))
      return emit(lexer, TokenType_And);
    exit(-1);
    break;
  case '|':
    if (match(lexer, '|'))
      return emit(lexer, TokenType_Or);
    exit(-1);
    break;

  default:
    if (isdigit(ch))
      return read_number(lexer);
    else if (isalpha(ch) || ch == '_')
      return read_identifier(lexer);
    return emit(lexer, TokenType_Error);
  }
}