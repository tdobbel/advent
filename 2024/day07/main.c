#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#include "arena.h"

enum Operation { Add, Multiply, Concatenate };

typedef enum Operation op_t;

b8 ispossible(u64 *nums, u64 n_num, u64 target, u32 nop, u64 value);
u64 apply_operation(op_t op, u64 a, u64 b);
u64 concat_nums(u64 a, u64 b);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  mem_arena *perm_arena = arena_create(KiB(500));

  string8 file = {0};
  str_read_file(perm_arena, &file, argv[1]);
  vector *line_vec = VEC_ARENA_CREATE(perm_arena, string8);
  str_split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;

  string8 spl[2] = {0};
  string8 sep = STR8_LIT(": ");
  vector *snums = VEC_ARENA_CREATE(perm_arena, string8);
  vector *nums = VEC_ARENA_CREATE(perm_arena, u64);

  u64 part1 = 0, part2 = 0;
  for (u64 i = 0; i < line_vec->size && lines[i].size > 0; ++i) {
    str_split_once(spl, lines[i], sep);
    u64 target = str_parse_unsigned(spl[0]);
    str_split_whitespace(snums, spl[1]);
    string8 *num8 = (string8 *)snums->data;
    nums->size = 0;
    u64 init_value = str_parse_unsigned(num8[0]);
    for (u64 inum = 1; inum < snums->size; ++inum) {
      VEC_PUSH(nums, u64, str_parse_unsigned(num8[inum]));
    }
    if (ispossible(nums->data, nums->size, target, 2, init_value)) {
      part1 += target;
      part2 += target;
      continue;
    }
    if (ispossible(nums->data, nums->size, target, 3, init_value))
      part2 += target;
  }

  printf("Part1: %lu\nPart2: %lu\n", part1, part2);

  arena_destroy(perm_arena);

  return EXIT_SUCCESS;
}

u64 concat_nums(u64 a, u64 b) {
  u64 c = b;
  while (b > 0) {
    a *= 10;
    b /= 10;
  }
  return a + c;
}

u64 apply_operation(op_t op, u64 a, u64 b) {
  switch (op) {
  case Add:
    return a + b;
  case Multiply:
    return a * b;
  case Concatenate:
    return concat_nums(a, b);
  }
}

b8 ispossible(u64 *nums, u64 n_num, u64 target, u32 nop, u64 value) {
  if (n_num == 0)
    return target == value;
  for (u32 op = 0; op < nop; ++op) {
    u64 vnext = apply_operation(op, value, nums[0]);
    if (ispossible(nums + 1, n_num - 1, target, nop, vnext))
      return 1;
  }
  return 0;
}
