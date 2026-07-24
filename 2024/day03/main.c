#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#include "arena.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  mem_arena *perm_arena = arena_create(KiB(500));

  string8 file = {0};
  str_read_file(perm_arena, &file, argv[1]);
  vector *line_vec = VEC_ARENA_CREATE(perm_arena, string8);
  str_split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;
  b8 active = 1;

  vector *vec = VEC_ARENA_CREATE(perm_arena, string8);
  regex_t regex;
  regmatch_t matches[3];
  const char *pattern = "mul\\(([0-9]+),([0-9]+)\\)";

  if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
    fprintf(stderr, "Could not compile regex :(\n");
    return EXIT_FAILURE;
  }

  char *text = ALLOC_ARRAY(perm_arena, char, file.size);

  u64 part1 = 0, part2 = 0;

  string8 on_flag = STR8_LIT("do");
  string8 off_flag = STR8_LIT("don't");

  for (u64 i = 0; i < line_vec->size && lines[i].size > 0; ++i) {
    str_split(vec, lines[i], STR8_LIT("()"));
    string8 *sub = (string8 *)vec->data;
    for (u64 j = 0; j < vec->size; ++j) {
      memcpy(text, sub[j].str, sub[j].size);
      text[sub[j].size] = '\0';
      char *str = text;
      while (regexec(&regex, str, 3, matches, 0) == 0) {
        string8 s1 = (string8){.str = (u8 *)str + matches[1].rm_so,
                               .size = matches[1].rm_eo - matches[1].rm_so};
        string8 s2 = (string8){.str = (u8 *)str + matches[2].rm_so,
                               .size = matches[2].rm_eo - matches[2].rm_so};
        u64 prod = str_parse_unsigned(s1) * str_parse_unsigned(s2);
        if (active)
          part2 += prod;
        part1 += prod;
        str += matches[2].rm_eo;
      }
      if (str_ends_with(sub[j], on_flag))
        active = 1;
      else if (str_ends_with(sub[j], off_flag))
        active = 0;
    }
  }

  printf("Part 1: %lu\nPart 2: %lu\n", part1, part2);

  regfree(&regex);
  arena_destroy(perm_arena);

  return EXIT_SUCCESS;
}
