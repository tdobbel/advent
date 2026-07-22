#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

int compare_fn(const void *a, const void *b) { return (*(u32 *)a - *(u32 *)b); }

int get_score(vector *vec, u32 value) {
  u32 cntr = 0;
  u32 *nums = (u32 *)vec->data;
  for (u64 i = 0; i < vec->size; ++i) {
    cntr += nums[i] == value ? 1 : 0;
  }
  return cntr * value;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Missing input file");
    return EXIT_FAILURE;
  }

  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);
  vector *line_vec = VEC_CREATE(string8);
  split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;

  vector *left_vec = VEC_CREATE(u32);
  vector *right_vec = VEC_CREATE(u32);

  vector *splitted_vec = VEC_CREATE(string8);
  u64 v;
  for (u64 i = 0; i < line_vec->size && lines[i].size > 0; ++i) {
    split_whitespace(splitted_vec, lines[i]);
    string8 *splitted = (string8 *)splitted_vec->data;
    str_parse_unsigned(&v, splitted[0]);
    VEC_PUSH(left_vec, u32, (u32)v);
    str_parse_unsigned(&v, splitted[1]);
    VEC_PUSH(right_vec, u32, (u32)v);
  }
  vector_free(splitted_vec);
  vector_sort(left_vec, compare_fn);
  vector_sort(right_vec, compare_fn);

  u32 part1 = 0, part2 = 0;
  u32 *right = (u32 *)right_vec->data;
  u32 *left = (u32 *)left_vec->data;

  for (u64 i = 0; i < right_vec->size; ++i) {
    part1 += right[i] > left[i] ? right[i] - left[i] : left[i] - right[i];
    part2 += get_score(right_vec, left[i]);
  }

  printf("Part 1: %u\n", part1);
  printf("Part 2: %u\n", part2);

  vector_free(left_vec);
  vector_free(right_vec);
  free(file.str);

  return EXIT_SUCCESS;
}
