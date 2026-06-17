#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

u64 argmax(string8 s) {
  u8 vmax = 0;
  u64 imax = 0;
  for (u64 i = 0; i < s.size; ++i) {
    if (s.str[i] <= vmax)
      continue;
    vmax = s.str[i];
    imax = i;
  }
  return imax;
}

u64 max_value(string8 s, u64 count) {
  u64 v = 0;
  string8 slice = s;
  for (u64 i = count; i > 0; --i) {
    u64 imax =
        argmax((string8){.str = slice.str, .size = slice.size - i + 1});
    v = v * 10 + (u64)(slice.str[imax] - '0');
    slice =
        (string8){.str = slice.str + imax + 1, .size = slice.size - imax - 1};
  }
  return v;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "Could not open file %s\n", argv[1]);
    return EXIT_FAILURE;
  }
  char buffer[1024];
  u64 part1 = 0;
  u64 part2 = 0;
  while (fgets(buffer, 1024, fp)) {
    u64 n = strcspn(buffer, "\n");
    string8 s = (string8){.str = (u8 *)buffer, .size = n};
    part1 += max_value(s, 2);
    part2 += max_value(s, 12);
  }
  fclose(fp);

  printf("Part 1: %lu\n", part1);
  printf("Part 2: %lu\n", part2);

  return EXIT_SUCCESS;
}
