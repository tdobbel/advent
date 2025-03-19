#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

enum Operation { Add, Multiply, Concatenate };

const size_t MAX_LINE_LENGTH = 256;
const size_t MAX_LIST_LENGTH = 12;

uint64_t concatenate(uint64_t a, uint64_t b) {
  int c = b;
  while (b > 0) {
    a *= 10;
    b /= 10;
  }
  return a + c;
}

uint64_t executeOperation(uint64_t a, uint64_t b, enum Operation op) {
  switch (op) {
  case Add:
    return a + b;
  case Multiply:
    return a * b;
  case Concatenate:
    return concatenate(a, b);
  }
}

int checkLine(uint64_t *numbers, int size, int index, int nop, uint64_t accum) {
  if (accum > numbers[0]) {
    return 0;
  }
  if (index == size) {
    return accum == numbers[0];
  }
  int result = 0;
  for (int op = 0; op < nop; ++op) {
    uint64_t newAccum = executeOperation(accum, numbers[index], op);
    result += checkLine(numbers, size, index + 1, nop, newAccum);
  }
  return result;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Filename required");
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    printf("File '%s' not found \n", argv[1]);
    return 1;
  }

  char line[MAX_LINE_LENGTH];
  uint64_t numbers[MAX_LIST_LENGTH];
  uint64_t part1 = 0;
  uint64_t part2 = 0;
  while (fgets(line, MAX_LINE_LENGTH, file)) {
    line[strcspn(line, "\n")] = 0;
    int n = 0;
    char *token = strtok(line, " ");
    while (token != NULL) {
      if (n == 0) {
        token[strcspn(token, ":")] = '\0';
      }
      numbers[n++] = strtoul(token, NULL, 10);
      token = strtok(NULL, " ");
    }
    if (checkLine(numbers, n, 2, 2, numbers[1]))
      part1 += numbers[0];
    if (checkLine(numbers, n, 2, 3, numbers[1]))
      part2 += numbers[0];
  }
  printf("Part 1: %lu\n", part1);
  printf("Part 2: %lu\n", part2);
  fclose(file);
  return 0;
}
