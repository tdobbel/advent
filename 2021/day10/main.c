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
  for (u64 i = 0; i < line_vec->size; ++i) {
    u64 error = valid_line(lines[i], vec);
    if (error == 0) {
      // Part 2
    }
    part1 += error;
  }

  printf("Part 1: %lu\n", part1);

  vector_free(vec);
  free(file.str);

  return EXIT_SUCCESS;
}
