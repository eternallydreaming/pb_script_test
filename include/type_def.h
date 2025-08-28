#pragma once

#include "lexer.h"
#include <stdbool.h>

typedef enum ValueType {
  ValueType_Error = -2,
  ValueType_Void,
  ValueType_Null = 0,
  ValueType_Number,
  ValueType_Boolean,
  ValueType_String,
} ValueType;

typedef struct TypeDef {
  ValueType value : 8;
  bool optional : 1;
} TypeDef;

static const TypeDef TypeDef_Void = {ValueType_Void};
static const TypeDef TypeDef_Null = {ValueType_Null};
static const TypeDef TypeDef_Number = {ValueType_Number};
static const TypeDef TypeDef_Boolean = {ValueType_Boolean};
static const TypeDef TypeDef_String = {ValueType_String};

TypeDef new_type_def(ValueType value, bool optional);

bool is_type_def_void(TypeDef def);
bool is_type_def_null(TypeDef def);
bool is_type_def_number(TypeDef def);
bool is_type_def_boolean(TypeDef def);
bool is_type_def_string(TypeDef def);

bool compare_type_def(TypeDef lhs, TypeDef rhs);

TypeDef parse_type_def(Lexer *lexer);