#pragma once

#include "chunk.h"
#include "registry.h"

Chunk compile_script(const char *source, const Registry *registry);