#include "value.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint32_t active_values = 0;

uint32_t get_active_values() { return active_values; }

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

Value new_string_value_move(char *chars, uint32_t len) {
  ObjString *string = malloc(sizeof(*string));
  assert(string != NULL);
  string->ref_count = 1;
  string->len = len;
  string->chars = chars;
  active_values++;

  return (Value){
      .type = ValueType_String,
      .string = string,
  };
}

Value new_string_value(const char *chars, uint32_t len) {
  char *chars_copy = strndup(chars, len);
  assert(chars_copy != NULL);
  return new_string_value_move(chars_copy, len);
}

void reference_value(Value value) {
  if (is_value_primitive(value))
    return;

  value.object->ref_count++;
}

void release_value(Value value) {
  if (is_value_primitive(value))
    return;

  value.object->ref_count--;
  if (value.object->ref_count == 0) {
    active_values--;
    switch (value.type) {
    case ValueType_String:
      free(value.string->chars);
      break;
    default:
      break;
    }

    free(value.object);
  }
}

Value copy_value(Value value) {
  if (is_value_primitive(value))
    return value;

  switch (value.type) {
  case ValueType_String:
    return new_string_value(value.string->chars, value.string->len);
  default:
    assert(0);
  }
}

bool is_value_primitive(Value value) {
  switch (value.type) {
  case ValueType_Null:
  case ValueType_Number:
  case ValueType_Boolean:
    return true;
  default:
    return false;
  }
}

double value_as_number(Value value) {
  assert(value.type == ValueType_Number);
  return value.number;
}

bool value_as_boolean(Value value) {
  assert(value.type == ValueType_Boolean);
  return value.boolean;
}

ObjString *value_as_string(Value value) {
  assert(value.type == ValueType_String);
  return value.string;
}

const char *value_as_c_string(Value value) {
  return value_as_string(value)->chars;
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
  case ValueType_String:
    if (lhs.string->len != rhs.string->len)
      return false;
    return memcmp(lhs.string->chars, rhs.string->chars, lhs.string->len) == 0;
  }
  return false;
}

Value value_concat(Value lhs_value, Value rhs_value) {
  assert(lhs_value.type == ValueType_String &&
         rhs_value.type == ValueType_String);
  ObjString *lhs = lhs_value.string, *rhs = rhs_value.string;

  uint32_t len = lhs->len + rhs->len;
  char *chars = malloc(len + 1);
  assert(chars != NULL);
  memcpy(chars, lhs->chars, lhs->len);
  memcpy(chars + lhs->len, rhs->chars, rhs->len);
  chars[len] = 0;

  return new_string_value_move(chars, len);
}