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