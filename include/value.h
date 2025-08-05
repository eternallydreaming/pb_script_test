#pragma once

typedef enum ValueType {
  ValueType_Number = 0,
} ValueType;

typedef struct Value {
  ValueType type;
  union {
    double number;
  };
} Value;

Value new_number_value(double number);

double value_as_number(Value value);