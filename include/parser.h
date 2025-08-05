#pragma once

#include "bytecode.h"
#include "lexer.h"

Chunk compile_script(const char *source);