#include "stdio.h"
#include "stdlib.h"
#include <string.h>

const size_t MAX_LINE_LENGTH = 1024;
const size_t MAX_LINES = 2048;

int compare(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return 1;
  }
  int *values = (int *)malloc(MAX_LINES * sizeof(int));
  int cntr = 0;
  FILE *file = fopen(argv[1], "r");
  char line[MAX_LINE_LENGTH];
  int value = 0;
  while (fgets(line, MAX_LINE_LENGTH, file)) {
    if (cntr >= MAX_LINES) {
      printf("Too many lines\n");
      return 1;
    }
    if (strlen(line) == 1) {
      values[cntr++] = value;
      value = 0;
    } else {
      value += atoi(line);
    }
  }
  if (value > 0) {
    values[cntr++] = value;
  }
  fclose(file);
  values = realloc(values, cntr * sizeof(int));
  qsort(values, cntr, sizeof(int), compare);

  printf("Part 1: %d\n", values[cntr - 1]);
  printf("Part 2: %d\n",
         values[cntr - 1] + values[cntr - 2] + values[cntr - 3]);

  free(values);
  return 0;
}
