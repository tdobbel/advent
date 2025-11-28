#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 16

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return EXIT_FAILURE;
  }
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    printf("Error opening file\n");
    return EXIT_FAILURE;
  }
  char buffer[BUFFER_SIZE];
  int values[3] = {0, 0, 0};
  int iln = 0, prev = 0, sum1 = 0, sum2 = 0;
  int part1 = 0, part2 = 0;
  while (fgets(buffer, BUFFER_SIZE, fp)) {
    int depth = 0;
    int i = 0;
    while (buffer[i] != '\n') {
      depth = 10 * depth + buffer[i++] - '0';
    }
    iln++;
    for (int i = 0; i < 3; ++i) {
      values[i] += i < iln ? depth : 0;
    }
    if (iln > 1 && depth > prev)
      part1++;
    prev = depth;
    if (iln < 3)
      continue;
    sum1 = sum2;
    sum2 = values[0];
    for (int i = 0; i < 2; i++) {
      values[i] = values[i + 1];
    }
    values[2] = 0;
    if (iln > 3 && sum2 > sum1)
      part2++;
  }
  printf("Part 1: %d\n", part1);
  printf("Part 2: %d\n", part2);

  return EXIT_SUCCESS;
}
