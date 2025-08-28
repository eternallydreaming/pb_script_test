#pragma once

#include <stdbool.h>
#include <stddef.h>

bool compare_string(const char *lhs, size_t lhs_len, const char *rhs,
                    size_t rhs_len);