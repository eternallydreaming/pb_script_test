#include "utility.h"
#include <stdbool.h>
#include <string.h>

bool compare_string(const char *lhs, size_t lhs_len, const char *rhs,
                    size_t rhs_len) {
  if (lhs_len != rhs_len)
    return false;
  return memcmp(lhs, rhs, lhs_len) == 0;
}