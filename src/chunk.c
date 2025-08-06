#include "chunk.h"
#include <assert.h>
#include <stddef.h>
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

#define READ_CHUNK_FN(T, postfix)                                              \
  T read_chunk_##postfix(const Chunk *chunk, size_t *pos) {                    \
    assert(chunk->size >= sizeof(T) && *pos <= chunk->size - sizeof(T));       \
                                                                               \
    T result;                                                                  \
    memcpy(&result, chunk->code + *pos, sizeof(T));                            \
    (*pos) += sizeof(T);                                                       \
    return result;                                                             \
  }

READ_CHUNK_FN(int8_t, i8)
READ_CHUNK_FN(uint8_t, u8)
READ_CHUNK_FN(int16_t, i16)
READ_CHUNK_FN(uint16_t, u16)
READ_CHUNK_FN(int32_t, i32)
READ_CHUNK_FN(uint32_t, u32)
READ_CHUNK_FN(uint64_t, u64)
READ_CHUNK_FN(float, f32)
READ_CHUNK_FN(double, f64)

#define WRITE_CHUNK_FN(T, postfix)                                             \
  void write_chunk_##postfix(Chunk *chunk, T value) {                          \
    if (chunk->size + sizeof(T) > chunk->cap) {                                \
      if (chunk->cap == 0)                                                     \
        chunk->cap = 1;                                                        \
      while (chunk->cap < chunk->size + sizeof(T))                             \
        chunk->cap *= 2;                                                       \
                                                                               \
      chunk->code = realloc(chunk->code, chunk->cap * sizeof(*chunk->code));   \
      assert(chunk->code != NULL);                                             \
    }                                                                          \
                                                                               \
    memcpy(chunk->code + chunk->size, &value, sizeof(T));                      \
    chunk->size += sizeof(T);                                                  \
  }

WRITE_CHUNK_FN(int8_t, i8)
WRITE_CHUNK_FN(uint8_t, u8)
WRITE_CHUNK_FN(int16_t, i16)
WRITE_CHUNK_FN(uint16_t, u16)
WRITE_CHUNK_FN(int32_t, i32)
WRITE_CHUNK_FN(uint32_t, u32)
WRITE_CHUNK_FN(uint64_t, u64)
WRITE_CHUNK_FN(float, f32)
WRITE_CHUNK_FN(double, f64)

size_t write_chunk_hole(Chunk *chunk, size_t bits) {
  size_t pos = chunk->size;
  for (size_t i = 0; i < bits / 8; i++)
    write_chunk_u8(chunk, 0);
  return pos;
}

void patch_chunk_hole_u16(Chunk *chunk, size_t pos) {
  size_t patch_pos = chunk->size;
  chunk->size = pos; // why not?
  write_chunk_u16(chunk, patch_pos - pos - sizeof(uint16_t));
  chunk->size = patch_pos;
}