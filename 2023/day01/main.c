#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return 1;
  }

  char *numbers[9] = {"one", "two",   "three", "four", "five",
                      "six", "seven", "eight", "nine"};
  int values[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  FILE *file = fopen(argv[1], "r");
  int part1 = 0, part2 = 0;
  char line[BUFFER_SIZE];
  while (fgets(line, BUFFER_SIZE, file)) {
    int n = strcspn(line, "\n");
    line[n] = '\0';
    int start = 0;
    while (line[start] < '1' || line[start] > '9')
      start++;
    int x1 = line[start] - '0';
    int x2 = x1;
    int found = 0;
    for (int i = 1; i < start && !found; ++i) {
      char slice[i + 1];
      strncpy(slice, line, i + 1);
      for (int j = 0; j < 9; ++j) {
        if (strstr(slice, numbers[j])) {
          x2 = values[j];
          found = 1;
          break;
        }
      }
    }
    int end = n - 1;
    while (line[end] < '1' || line[end] > '9')
      end--;
    int y1 = line[end] - '0';
    int y2 = y1;
    found = 0;
    for (int i = n - 2; i > end && !found; --i) {
      for (int j = 0; j < 9; ++j) {
        if (strstr(line + i, numbers[j])) {
          y2 = values[j];
          found = 1;
          break;
        }
      }
    }
    part1 += x1 * 10 + y1;
    part2 += x2 * 10 + y2;
  }
  fclose(file);

  printf("Part 1: %d\n", part1);
  printf("Part 2: %d\n", part2);

  return 0;
}
