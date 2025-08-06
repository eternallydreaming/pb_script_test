#include "value.h"
#include <assert.h>
#include <stdbool.h>

Value new_null_value() {
  return (Value){
      .type = ValueType_Null,
  };
}

Value new_number_value(double number) {
  return (Value){
      .type = ValueType_Number,
      .number = number,
  };
}

Value new_boolean_value(bool boolean) {
  return (Value){
      .type = ValueType_Boolean,
      .boolean = boolean,
  };
}

Value copy_value(Value value) { return value; }

double value_as_number(Value value) {
  assert(value.type == ValueType_Number);
  return value.number;
}

bool value_as_boolean(Value value) {
  assert(value.type == ValueType_Boolean);
  return value.boolean;
}

bool value_compare(Value lhs, Value rhs) {
  if (lhs.type != rhs.type)
    return false;
  switch (lhs.type) {
  case ValueType_Null:
    return true;
  case ValueType_Number:
    return lhs.number == rhs.number;
  case ValueType_Boolean:
    return lhs.boolean = rhs.boolean;
  }
}