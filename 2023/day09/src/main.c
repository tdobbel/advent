#include "stdio.h"
#include "stdlib.h"
#include "string.h"

const size_t MAX_LINE_LENGTH = 200;
const size_t MAX_ARRAY_SIZE = 50;

void parse_line(char *line, int *numbers, int *size) {
  char *token = strtok(line, " ");
  int cntr = 0;
  while (token != NULL) {
    if (cntr >= MAX_ARRAY_SIZE) {
      printf("Array size exceeded\n");
      exit(1);
    }
    numbers[cntr++] = atoi(token);
    token = strtok(NULL, " ");
  }
  *size = cntr;
}

void process_line(int *numbers, int size, int *part1, int *part2) {
  int past = 0;
  int future = 0;
  int stop = 0;
  int factor = 1;
  while (stop == 0) {
    past += numbers[0] * factor;
    future += numbers[size - 1];
    stop = 1;
    for (int i = 0; i < size - 1; ++i) {
      numbers[i] = numbers[i + 1] - numbers[i];
      if (numbers[i] != 0) {
        stop = 0;
      }
    }
    factor *= -1;
    size--;
  }
  *part1 += future;
  *part2 += past;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
  }

  int part1 = 0;
  int part2 = 0;
  FILE *file = fopen(argv[1], "r");
  char line[MAX_LINE_LENGTH];
  int numbers[MAX_ARRAY_SIZE];
  int size;
  while (fgets(line, MAX_LINE_LENGTH, file)) {
    parse_line(line, numbers, &size);
    process_line(numbers, size, &part1, &part2);
  }
  fclose(file);
  printf("Part 1: %d\n", part1);
  printf("Part 2: %d\n", part2);

  return 0;
}
