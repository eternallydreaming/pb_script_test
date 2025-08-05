#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct Chunk {
  size_t size, cap;
  uint8_t *code;
} Chunk;

Chunk new_chunk();
void delete_chunk(Chunk *chunk);

uint8_t read_chunk_u8(const Chunk *chunk, size_t *pos);
uint64_t read_chunk_u64(const Chunk *chunk, size_t *pos);
double read_chunk_f64(const Chunk *chunk, size_t *pos);

void write_chunk_u8(Chunk *chunk, uint8_t value);
void write_chunk_u64(Chunk *chunk, uint64_t value);
void write_chunk_f64(Chunk *chunk, double value);