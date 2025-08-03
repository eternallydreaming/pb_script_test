#include "lexer.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct Lexer {
  const char *source;
  size_t source_len;
  size_t pos;
} Lexer;

static Token emit(const Lexer *lexer, TokenType type) {
  return (Token){
      .type = type,
  };
}

static Token emit_number(const Lexer *lexer, TokenType type, double number) {
  return (Token){
      .type = type,
      .number = number,
  };
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

  double number = strtod(start, NULL);
  if (errno == ERANGE) {
    errno = 0;
    return emit(lexer, TokenType_Error);
  }
  return emit_number(lexer, TokenType_Number, number);
}

static Token advance_token(Lexer *lexer) {
  while (true) {
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
      if (isspace(ch)) {
        advance(lexer);
      } else if (isdigit(ch)) {
        return read_number(lexer);
      } else {
        advance(lexer);
        return emit(lexer, TokenType_Error);
      }
      break;
    }
  }
}

Token *lexer_tokenize(const char *source) {
  Lexer lexer = {
      .source = source,
      .source_len = strlen(source),
      .pos = 0,
  };

  Token *tokens = NULL;
  size_t tokens_num = 0, tokens_cap = 0;

  bool error = false;
  while (true) {
    Token token = advance_token(&lexer);
    if (token.type == TokenType_Error)
      error = true;
    if (error)
      continue;

    if (tokens_num >= tokens_cap) {
      if (tokens_cap == 0)
        tokens_cap = 1;
      tokens_cap *= 2;
      tokens = realloc(tokens, tokens_cap * sizeof(*tokens));
      assert(tokens != NULL);
    }
    tokens[tokens_num++] = token;

    if (token.type == TokenType_Eof)
      break;
  }

  if (error) {
    free(tokens);
    return NULL;
  }
  return tokens;
}