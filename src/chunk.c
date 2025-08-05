#include "chunk.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Chunk new_chunk() {
  return (Chunk){
      .size = 0,
      .cap = 0,
      .code = NULL,
  };
}

void delete_chunk(Chunk *chunk) { free(chunk->code); }

uint8_t read_chunk_u8(const Chunk *chunk, size_t *pos) {
  assert(*pos < chunk->size);
  return chunk->code[(*pos)++];
}

uint64_t read_chunk_u64(const Chunk *chunk, size_t *pos) {
  uint64_t result = 0;
  for (size_t i = 0; i < 8; i++)
    result |= (uint64_t)read_chunk_u8(chunk, pos) << (i * 8);
  return result;
}

double read_chunk_f64(const Chunk *chunk, size_t *pos) {
  uint64_t as_u64 = read_chunk_u64(chunk, pos);
  double result;
  memcpy(&result, &as_u64, sizeof(result));
  return result;
}

void write_chunk_u8(Chunk *chunk, uint8_t value) {
  if (chunk->size >= chunk->cap) {
    if (chunk->cap == 0)
      chunk->cap = 1;
    chunk->cap *= 2;
    chunk->code = realloc(chunk->code, chunk->cap * sizeof(*chunk->code));
    assert(chunk->code != NULL);
  }

  chunk->code[chunk->size++] = value;
}

void write_chunk_u64(Chunk *chunk, uint64_t value) {
  for (size_t i = 0; i < 8; i++)
    write_chunk_u8(chunk, value >> (i * 8));
}

void write_chunk_f64(Chunk *chunk, double value) {
  uint64_t as_u64;
  memcpy(&as_u64, &value, sizeof(value));
  write_chunk_u64(chunk, as_u64);
}