#pragma once

#include <stddef.h>

typedef enum TokenType {
  TokenType_Error = -1,
  TokenType_Eof = 0,

  TokenType_Plus,
  TokenType_Minus,
  TokenType_Star,
  TokenType_Slash,

  TokenType_Number,
  TokenType_String,
} TokenType;

typedef struct Token {
  TokenType type;
  union {
    double number;
    struct {
      const char *start;
      size_t len;
    } text;
  };
} Token;

typedef struct Lexer {
  const char *source;
  size_t source_len;
  size_t pos;

  Token token;
} Lexer;

Lexer new_lexer(const char *source);

Token lexer_peek(const Lexer *lexer);
Token lexer_advance(Lexer *lexer);
