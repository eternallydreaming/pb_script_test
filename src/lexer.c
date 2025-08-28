#include "lexer.h"
#include "utility.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Keyword {
  const char *text;
  TokenType type;
  LexerContext context;
} Keyword;

// clang-format off
static const Keyword KEYWORDS[] = {
    {"null",     TokenType_Null,     LexerContext_None},
    {"true",     TokenType_True,     LexerContext_None},
    {"false",    TokenType_False,    LexerContext_None},
    {"let",      TokenType_Let,      LexerContext_None},
    {"if",       TokenType_If,       LexerContext_None},
    {"else",     TokenType_Else,     LexerContext_None},
    {"while",    TokenType_While,    LexerContext_None},
    {"for",      TokenType_For,      LexerContext_None},
    {"in",       TokenType_In,       LexerContext_For},
    {"by",       TokenType_By,       LexerContext_For},
    {"break",    TokenType_Break,    LexerContext_None},
    {"continue", TokenType_Continue, LexerContext_None},
    {NULL, 0, 0},
};
// clang-format on

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

static Token emit_text(Lexer *lexer, TokenType type, size_t start_pos,
                       size_t len) {
  lexer->token = (Token){
      .type = type,
      .text.start = lexer->source + start_pos,
      .text.len = len,
  };
  return lexer->token;
}

static Token number(Lexer *lexer) {
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

static Token string(Lexer *lexer) {
  size_t start_pos = lexer->pos;
  while (peek(lexer) != '\r' && peek(lexer) != '\n' && peek(lexer) != '"')
    advance(lexer);
  size_t len = lexer->pos - start_pos;
  if (!match(lexer, '"')) {
    puts("expected '\"' to close '\"'");
    exit(-1);
  }

  return emit_text(lexer, TokenType_String, start_pos, len);
}

static Token identifier(Lexer *lexer) {
  size_t start_pos = lexer->pos - 1;
  while (isalnum(peek(lexer)) || peek(lexer) == '_')
    advance(lexer);
  size_t len = lexer->pos - start_pos;

  for (const Keyword *keyword = KEYWORDS; keyword->text != NULL; keyword++) {
    if (compare_string(lexer->source + start_pos, len, keyword->text,
                       strlen(keyword->text))) {
      if (keyword->context != LexerContext_None &&
          keyword->context != lexer->context) {
        // context-based keyword that didn't match the current context, treat
        // this as an identifier
        break;
      }
      return emit(lexer, keyword->type);
    }
  }
  return emit_text(lexer, TokenType_Identifier, start_pos, len);
}

Lexer new_lexer(const char *source) {
  Lexer lexer = {
      .source = source,
      .pos = 0,

      .context = LexerContext_None,

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
  case '{':
    return emit(lexer, TokenType_LBrace);
  case '}':
    return emit(lexer, TokenType_RBrace);
  case '[':
    return emit(lexer, TokenType_LBracket);
  case ']':
    return emit(lexer, TokenType_RBracket);

  case '.':
    if (isdigit(peek(lexer)))
      return number(lexer);
    if (match(lexer, '.')) {
      if (match(lexer, '.'))
        return emit(lexer, TokenType_TripleDot);
    }
    assert(0);
  case ',':
    return emit(lexer, TokenType_Comma);
  case ';':
    return emit(lexer, TokenType_Semicolon);

  case '+':
    return emit(lexer, TokenType_Plus);
  case '-':
    if (match(lexer, '>'))
      return emit(lexer, TokenType_Arrow);
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
    if (match(lexer, '>'))
      return emit(lexer, TokenType_FatArrow);
    if (match(lexer, '='))
      return emit(lexer, TokenType_Equal);
    return emit(lexer, TokenType_Assign);
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
    assert(0);
  case '|':
    if (match(lexer, '|'))
      return emit(lexer, TokenType_Or);
    assert(0);

  case '"':
    return string(lexer);

  default:
    if (isdigit(ch))
      return number(lexer);
    else if (isalpha(ch) || ch == '_')
      return identifier(lexer);
    return emit(lexer, TokenType_Error);
  }
}