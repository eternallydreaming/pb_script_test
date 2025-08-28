#pragma once

#include <stddef.h>

typedef enum TokenType {
  TokenType_Error = -1,
  TokenType_Eof = 0,

  TokenType_LParen,
  TokenType_RParen,
  TokenType_LBrace,
  TokenType_RBrace,
  TokenType_LBracket,
  TokenType_RBracket,

  TokenType_TripleDot,
  TokenType_Comma,
  TokenType_Semicolon,
  TokenType_Question,

  TokenType_Plus,
  TokenType_Minus,
  TokenType_Arrow,
  TokenType_Star,
  TokenType_Slash,

  TokenType_Assign,

  TokenType_FatArrow,
  TokenType_Equal,
  TokenType_Bang,
  TokenType_NotEqual,
  TokenType_Less,
  TokenType_LessEqual,
  TokenType_Greater,
  TokenType_GreaterEqual,

  TokenType_And,
  TokenType_Or,

  TokenType_Null,
  TokenType_True,
  TokenType_False,
  TokenType_Let,
  TokenType_If,
  TokenType_Else,
  TokenType_While,
  TokenType_For,
  TokenType_In,
  TokenType_By,
  TokenType_Break,
  TokenType_Continue,

  TokenType_Number,
  TokenType_Identifier,
  TokenType_String,
} TokenType;

typedef enum LexerContext {
  LexerContext_None = 0,
  LexerContext_For,
} LexerContext;

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
  size_t pos;

  LexerContext context;

  Token token;
} Lexer;

Lexer new_lexer(const char *source);

Token lexer_peek(const Lexer *lexer);
Token lexer_advance(Lexer *lexer);
