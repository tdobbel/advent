#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t MAX_LINE_SIZE = 256;
const size_t MAX_LINE = 256;

char getentry(const char *grid, int ncols, int i, int j) {
  return grid[i * ncols + j];
}

int isXmas(char *word) {
  if (strncmp(word, "XMAS", 4) == 0 || strncmp(word, "SAMX", 4) == 0) {
    return 1;
  }
  return 0;
}

void part1(const char *grid, int nrows, int ncols, int i, int j, int *count) {
  int k;
  char word[4];
  if (j <= ncols - 4) {
    for (k = 0; k < 4; ++k) {
      word[k] = getentry(grid, ncols, i, j + k);
    }
    if (isXmas(word))
      *count += 1;
  }
  if (i <= nrows - 4) {
    for (k = 0; k < 4; ++k) {
      word[k] = getentry(grid, ncols, i + k, j);
    }
    if (isXmas(word))
      *count += 1;
  }
  if (j >= 3 && i <= nrows - 4) {
    for (k = 0; k < 4; ++k) {
      word[k] = getentry(grid, ncols, i + k, j - k);
    }
    if (isXmas(word))
      *count += 1;
  }
  if (j <= ncols - 4 && i <= nrows - 4) {
    for (k = 0; k < 4; ++k) {
      word[k] = getentry(grid, ncols, i + k, j + k);
    }
    if (isXmas(word))
      *count += 1;
  }
}

int ismas(char *word) {
  if (strncmp(word, "MAS", 3) == 0 || strncmp(word, "SAM", 3) == 0) {
    return 1;
  }
  return 0;
}

void part2(const char *grid, int ncols, int i, int j, int *count) {
  char diag1[3];
  char diag2[3];
  for (int k = 0; k < 3; ++k) {
    diag1[k] = getentry(grid, ncols, i + k, j + k);
    diag2[k] = getentry(grid, ncols, i + k, j + 2 - k);
  }
  if (ismas(diag1) && ismas(diag2))
    *count += 1;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  char line[MAX_LINE_SIZE];
  int nrows = 0;
  int ncols = 0;
  int index = 0;
  char *grid = (char *)malloc(sizeof(char) * MAX_LINE_SIZE * MAX_LINE);
  while (fgets(line, MAX_LINE_SIZE, file)) {
    ncols = strcspn(line, "\n");
    for (int i = 0; i < ncols; ++i) {
      grid[index++] = line[i];
    }
    nrows++;
  }
  grid = (char *)realloc(grid, sizeof(char) * index);
  int count = 0;
  for (int i = 0; i < nrows; ++i) {
    for (int j = 0; j < ncols; ++j) {
      part1(grid, nrows, ncols, i, j, &count);
    }
  }
  printf("Part 1: %d\n", count);
  count = 0;
  for (int i = 0; i <= nrows - 3; ++i) {
    for (int j = 0; j <= ncols - 3; ++j) {
      part2(grid, ncols, i, j, &count);
    }
  }
  printf("Part 2: %d\n", count);
  fclose(file);
  free(grid);
  return 0;
}
