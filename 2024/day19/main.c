#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#include "arena.h"

#define MAX_TOWEL_SIZE 10

b8 str8_eql_fun(const void *a, const void *b) {
  return str_equal(*(string8 *)a, *(string8 *)b);
}

u64 count_possibilities(string8 design, vector **patterns, u32 max_size) {
  u64 *counter = malloc(design.size * sizeof(u64));
  for (u32 i = 0; i < design.size; ++i) {
    counter[i] = 0;
    for (u32 size = 1; size <= i + 1 && size <= max_size; ++size) {
      u32 start = i + 1 - size;
      string8 towel = (string8){.str=design.str + start, .size=size};
      if (!vector_contains(patterns[size - 1], &towel, str8_eql_fun))
        continue;
      if (i < size)
        counter[i]++;
      else
        counter[i] += counter[i - size];
    }
  }
  u64 n_ways = counter[design.size - 1];
  free(counter);
  return n_ways;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file");
    return EXIT_FAILURE;
  }
  string8 file = {0};
  mem_arena *perm_arena = arena_create(KiB(500));
  str_read_file(perm_arena, &file, argv[1]);
  vector *line_vec = VEC_ARENA_CREATE(perm_arena, string8);
  split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;

  vector **patterns = ALLOC_ARRAY(perm_arena, vector *, MAX_TOWEL_SIZE);
  for (u64 i = 0; i < MAX_TOWEL_SIZE; ++i) {
    patterns[i] = VEC_ARENA_CREATE(perm_arena, string8);
  }

  vector *towel_vec = VEC_CREATE(string8);
  split(towel_vec, lines[0], STR8_LIT(", "));
  string8 *towels = (string8 *)towel_vec->data;
  u32 max_size = 0;
  for (u32 i = 0; i < towel_vec->size; ++i) {
    u32 size = towels[i].size;
    if (size > MAX_TOWEL_SIZE) {
      fprintf(stderr, "towel larger than expected: %u > %u", size,
              MAX_TOWEL_SIZE);
      return EXIT_FAILURE;
    }
    if (size > max_size)
      max_size = size;
    VEC_PUSH(patterns[size - 1], string8, towels[i]);
  }
  vector_free(towel_vec);

  u64 part1 = 0;
  u64 part2 = 0;
  for (u64 i = 2; i < line_vec->size; ++i) {
    u64 n_ways = count_possibilities(lines[i], patterns, max_size);
    part1 += MIN(n_ways, 1);
    part2 += n_ways;
  }
  printf("Part 1: %lu\n", part1);
  printf("Part 2: %lu\n", part2);

  arena_destroy(perm_arena);

  return EXIT_SUCCESS;
}
