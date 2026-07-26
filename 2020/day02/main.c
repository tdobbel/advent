#include <assert.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#define BUFSIZE 512

#define STR8_GRP(r, s)                                                         \
  (string8) { .str = (s).str + (r).rm_so, .size = (r).rm_eo - (r).rm_so }

typedef struct {
  u64 cmin, cmax;
  u8 c;
} pwd_rule;

void str8_to_char(char *dst, const string8 src);
b8 valid_password1(pwd_rule rule, string8 password);
b8 valid_password2(pwd_rule rule, string8 password);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);
  vector *line_vec = VEC_CREATE(string8);
  str_split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;

  regex_t regex;
  regmatch_t matches[5];
  const char *pattern = "([0-9]+)-([0-9]+) ([a-z]): ([a-z]+)";

  if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
    fprintf(stderr, "Could not compile regex :(\n");
    return EXIT_FAILURE;
  }

  char buffer[BUFSIZE];

  u32 part1 = 0;
  u32 part2 = 0;
  for (u64 i = 0; i < line_vec->size && lines[i].size > 0; ++i) {
    str8_to_char(buffer, lines[i]);
    assert(regexec(&regex, buffer, 5, matches, 0) == 0);
    u64 cmin = str_parse_unsigned(STR8_GRP(matches[1], lines[i]));
    u64 cmax = str_parse_unsigned(STR8_GRP(matches[2], lines[i]));
    u8 c = lines[i].str[matches[3].rm_so];
    string8 pwd = STR8_GRP(matches[4], lines[i]);
    pwd_rule rule = (pwd_rule){.c = c, .cmin = cmin, .cmax = cmax};
    if (valid_password1(rule, pwd))
      part1++;
    if (valid_password2(rule, pwd))
      part2++;
  }

  printf("Part 1: %u\nPart 2: %u\n", part1, part2);

  regfree(&regex);
  vector_free(line_vec);
  free(file.str);

  return EXIT_SUCCESS;
}

void str8_to_char(char *dst, const string8 src) {
  assert(src.size < BUFSIZE);
  memcpy(dst, src.str, src.size);
  dst[src.size] = '\0';
}

b8 valid_password1(pwd_rule rule, string8 password) {
  u32 cntr = 0;
  for (u64 i = 0; i < password.size; ++i) {
    if (password.str[i] == rule.c)
      cntr++;
  }
  return cntr >= rule.cmin && cntr <= rule.cmax;
}

b8 valid_password2(pwd_rule rule, string8 password) {
  return (password.str[rule.cmin - 1] == rule.c) ^
         (password.str[rule.cmax - 1] == rule.c);
}
