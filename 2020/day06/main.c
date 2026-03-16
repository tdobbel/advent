#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint32_t u32;
typedef uint8_t u8;

#define BUFSIZE 64

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "Could not open input file %s\n", argv[1]);
    return EXIT_FAILURE;
  }
  char buffer[BUFSIZE];
  u32 part1 = 0;
  u32 part2 = 0;

  u32 answers[26];
  u32 groupsize = 0;
  memset(answers, 0, 26 * sizeof(u32));

  while (fgets(buffer, BUFSIZE, fp)) {
    u32 n = strcspn(buffer, "\n");
    if (n == 0) {
      for (u32 i = 0; i < 26; ++i) {
        part1 += answers[i] > 0;
        part2 += answers[i] == groupsize;
      }
      memset(answers, 0, 26 * sizeof(u32));
      groupsize = 0;
      continue;
    }
    groupsize++;
    for (u32 i = 0; i < n; ++i) {
      u8 c = buffer[i];
      answers[c - 'a']++;
    }
  }
  for (u32 i = 0; i < 26; ++i) {
    part1 += answers[i] > 0;
    part2 += answers[i] == groupsize;
  }

  printf("Part 1: %u\n", part1);
  printf("Part 2: %u\n", part2);

  fclose(fp);
  return EXIT_SUCCESS;
}
