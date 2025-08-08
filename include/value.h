#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum ValueType {
  ValueType_Null = 0,
  ValueType_Number,
  ValueType_Boolean,
  ValueType_String,
} ValueType;

typedef struct Obj {
  uint16_t ref_count;
} Obj;

typedef struct ObjString {
  uint16_t ref_count;
  uint32_t len;
  char *chars;
} ObjString;

typedef struct Value {
  ValueType type;
  union {
    double number;
    bool boolean;
    Obj *object;
    ObjString *string;
  };
} Value;

uint32_t get_active_values(); // ref counter test

Value new_null_value();
Value new_number_value(double number);
Value new_boolean_value(bool boolean);
Value new_string_value_move(char *chars, uint32_t len);
Value new_string_value(const char *chars, uint32_t len);

void reference_value(Value value);
void release_value(Value value);

Value copy_value(Value value);

bool is_value_primitive(Value value);
double value_as_number(Value value);
bool value_as_boolean(Value value);
ObjString *value_as_string(Value value);
const char *value_as_c_string(Value value);

bool value_compare(Value lhs, Value rhs);

Value value_concat(Value lhs, Value rhs);
