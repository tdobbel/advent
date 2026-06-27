#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

b8 closing_char(u8 c) { return c == ')' || c == '}' || c == ']' || c == '>'; }

b8 get_closing(u8 c) {
  if (c == '(')
    return ')';
  if (c == '[')
    return ']';
  if (c == '{')
    return '}';
  return '>';
}

u64 get_error_score(u8 c) {
  if (c == ')')
    return 3;
  if (c == ']')
    return 57;
  if (c == '}')
    return 1197;
  return 25137;
}

u64 get_completion_score(u8 c) {
  if (c == ')')
    return 1;
  if (c == ']')
    return 2;
  if (c == '}')
    return 3;
  return 4;
}

int compare_fn(const void *a, const void *b) {
  u64 xa = *(u64 *)a;
  u64 xb = *(u64 *)b;
  if (xa < xb)
    return -1;
  if (xa > xb)
    return 1;
  return 0;
}

u64 valid_line(string8 line, vector *vec) {
  vec->size = 0;
  for (u64 i = 0; i < line.size; ++i) {
    u8 c = line.str[i];
    if (!closing_char(c)) {
      VEC_PUSH(vec, u8, c);
      continue;
    }
    u8 expected = get_closing(*(u8 *)vector_pop(vec));
    if (expected != c)
      return get_error_score(c);
  }
  return 0;
}

u64 complete_line(string8 line, vector *vec) {
  vec->size = 0;
  for (u64 i = 0; i < line.size; ++i) {
    u8 c = line.str[i];
    if (!closing_char(c)) {
      VEC_PUSH(vec, u8, c);
      continue;
    }
    u8 clast = *(u8 *)vector_get_last(vec);
    assert(c == get_closing(clast));
    vector_pop(vec);
  }
  u64 score = 0;
  while (vec->size > 0) {
    score *= 5;
    u8 open_char = *(u8 *)vector_pop(vec);
    score += get_completion_score(get_closing(open_char));
  }
  return score;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
  }
  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);

  vector *line_vec = VEC_CREATE(string8);
  split(line_vec, file, STR8_LIT("\n"));
  u64 part1 = 0;
  string8 *lines = (string8 *)line_vec->data;
  vector *vec = VEC_CREATE(u8);
  vector *score_vec = VEC_CREATE(u64);
  for (u64 i = 0; i < line_vec->size; ++i) {
    u64 error = valid_line(lines[i], vec);
    if (error == 0) {
      u64 score = complete_line(lines[i], vec);
      VEC_PUSH(score_vec, u64, score);
    }
    part1 += error;
  }
  vector_sort(score_vec, compare_fn);
  u64 *scores = (u64 *)score_vec->data;
  u64 part2 = scores[score_vec->size / 2];

  printf("Part 1: %lu\n", part1);
  printf("Part 2: %lu\n", part2);

  vector_free(vec);
  vector_free(score_vec);
  free(file.str);

  return EXIT_SUCCESS;
}
