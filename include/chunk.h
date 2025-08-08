#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct ChunkString {
  uint32_t len;
  char *chars;
} ChunkString;

typedef struct Chunk {
  size_t strings_num;
  ChunkString *strings;

  size_t size, cap;
  uint8_t *code;
} Chunk;

Chunk new_chunk();
void delete_chunk(Chunk *chunk);

size_t add_chunk_string(Chunk *chunk, const char *chars, uint32_t len);

int8_t read_chunk_i8(const Chunk *chunk, size_t *pos);
uint8_t read_chunk_u8(const Chunk *chunk, size_t *pos);
int16_t read_chunk_i16(const Chunk *chunk, size_t *pos);
uint16_t read_chunk_u16(const Chunk *chunk, size_t *pos);
int32_t read_chunk_i32(const Chunk *chunk, size_t *pos);
uint32_t read_chunk_u32(const Chunk *chunk, size_t *pos);
uint64_t read_chunk_u64(const Chunk *chunk, size_t *pos);
float read_chunk_f32(const Chunk *chunk, size_t *pos);
double read_chunk_f64(const Chunk *chunk, size_t *pos);

void write_chunk_i8(Chunk *chunk, int8_t value);
void write_chunk_u8(Chunk *chunk, uint8_t value);
void write_chunk_i16(Chunk *chunk, int16_t value);
void write_chunk_u16(Chunk *chunk, uint16_t value);
void write_chunk_i32(Chunk *chunk, int32_t value);
void write_chunk_u32(Chunk *chunk, uint32_t value);
void write_chunk_u64(Chunk *chunk, uint64_t value);
void write_chunk_f32(Chunk *chunk, float value);
void write_chunk_f64(Chunk *chunk, double value);

size_t write_chunk_hole(Chunk *chunk, size_t bits);
void patch_chunk_hole_u16(Chunk *chunk, size_t pos);