#include "value.h"
#include <assert.h>

Value new_number_value(double number) {
  return (Value){
      .type = ValueType_Number,
      .number = number,
  };
}

double value_as_number(Value value) {
  assert(value.type == ValueType_Number);
  return value.number;
}