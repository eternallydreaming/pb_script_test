#include "type_def.h"
#include "lexer.h"
#include "utility.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct BuiltinType {
  const char *text;
  ValueType value;
} BuiltinType;

// clang-format off
static const BuiltinType BUILTIN_TYPES[] = {
    {"void",   ValueType_Void},
    {"null",   ValueType_Null},
    {"number", ValueType_Number},
    {"bool",   ValueType_Boolean},
    {"string", ValueType_String},
    {NULL,     0},
};
// clang-format on

TypeDef new_type_def(ValueType value, bool optional) {
  return (TypeDef){
      .value = value,
      .optional = optional,
  };
}

bool is_type_def_void(TypeDef def) { return def.value == ValueType_Void; }
bool is_type_def_null(TypeDef def) { return def.value == ValueType_Null; }
bool is_type_def_number(TypeDef def) { return def.value == ValueType_Number; }
bool is_type_def_boolean(TypeDef def) { return def.value == ValueType_Boolean; }
bool is_type_def_string(TypeDef def) { return def.value == ValueType_String; }

bool compare_type_def(TypeDef lhs, TypeDef rhs) {
  if (lhs.optional && rhs.value == ValueType_Null)
    return true;
  if (lhs.value == rhs.value) {
    switch (lhs.value) {
    case ValueType_Error:
    case ValueType_Void:
    case ValueType_Null:
    case ValueType_Number:
    case ValueType_Boolean:
    case ValueType_String:
      return true;
    }
  }
  return false;
}

TypeDef parse_type_def(Lexer *lexer) {
  Token token = lexer_peek(lexer);
  if (token.type != TokenType_Identifier) {
    puts("expected identifier");
    return new_type_def(ValueType_Error, false);
  }
  lexer_advance(lexer);

  TypeDef def = new_type_def(ValueType_Error, false);
  for (const BuiltinType *type = BUILTIN_TYPES; type->text != NULL; type++) {
    if (compare_string(token.text.start, token.text.len, type->text,
                       strlen(type->text))) {
      def.value = type->value;
      break;
    }
  }

  if (lexer_peek(lexer).type == TokenType_Question) {
    // optional type
    lexer_advance(lexer);
    def.optional = true;
  }

  return def;
}