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
} TokenType;

typedef struct Token {
  TokenType type;
  union {
    double number;
    char *string;
  };
} Token;

Token *tokenize_script(const char *source);
