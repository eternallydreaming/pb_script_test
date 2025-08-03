#include "lexer.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  Token *tokens = lexer_tokenize("2 + 9 * 3");
  Token *token = tokens;
  while (token->type != TokenType_Eof) {
    switch (token->type) {
    case TokenType_Plus:
      puts("TokenType_Plus");
      break;
    case TokenType_Minus:
      puts("TokenType_Minus");
      break;
    case TokenType_Star:
      puts("TokenType_Star");
      break;
    case TokenType_Slash:
      puts("TokenType_Slash");
      break;
    case TokenType_Number:
      printf("TokenType_Number: %f\n", token->number);
      break;
    default:
      break;
    }
    token++;
  }
  return 0;
}