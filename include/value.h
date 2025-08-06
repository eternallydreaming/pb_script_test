#pragma once

#include <stdbool.h>

typedef enum ValueType {
  ValueType_Null = 0,
  ValueType_Number,
  ValueType_Boolean,
} ValueType;

typedef struct Value {
  ValueType type;
  union {
    double number;
    bool boolean;
  };
} Value;

Value new_null_value();
Value new_number_value(double number);
Value new_boolean_value(bool boolean);

Value copy_value(Value value);

double value_as_number(Value value);
bool value_as_boolean(Value value);

bool value_compare(Value lhs, Value rhs);