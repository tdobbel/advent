#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define MAX_LINE_SIZE 64
#define MAX_LINES 64

void findTrails(const int *grid, int nrows, int ncols, int index, int *trails) {
  int current = grid[index];
  if (current == 9) {
    trails[index] += 1;
    return;
  }
  int shiftx[4] = {0, 1, 0, -1};
  int shifty[4] = {1, 0, -1, 0};
  int y0 = index / ncols;
  int x0 = index % ncols;
  int x, y;
  for (int k = 0; k < 4; k++) {
    x = x0 + shiftx[k];
    y = y0 + shifty[k];
    if (x < 0 || x >= ncols || y < 0 || y >= nrows) {
      continue;
    }
    int index_ = y * ncols + x;
    if (grid[index_] == current + 1) {
      findTrails(grid, nrows, ncols, index_, trails);
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  char line[MAX_LINE_SIZE];
  int nrows = 0;
  int ncols;
  int nstart = 0;
  int *start = (int *)malloc(MAX_LINES * MAX_LINE_SIZE * sizeof(int));
  int *grid = (int *)malloc(MAX_LINES * MAX_LINE_SIZE * sizeof(int));
  int index = 0;
  while (fgets(line, MAX_LINE_SIZE, file)) {
    ncols = strcspn(line, "\n");
    for (int j = 0; j < ncols; ++j) {
      char number[2] = {line[j], '\0'};
      grid[index] = atoi(number);
      if (grid[index] == 0) {
        start[nstart++] = index;
      }
      index++;
    }
    nrows++;
  }
  fclose(file);

  start = (int *)realloc(start, nstart * sizeof(int));
  grid = (int *)realloc(grid, nrows * ncols * sizeof(int));
  int *trails = calloc(sizeof(int), nrows * ncols);

  int part1 = 0;
  int part2 = 0;
  for (int i = 0; i < nstart; ++i) {
    findTrails(grid, nrows, ncols, start[i], trails);
    for (int j = 0; j < nrows * ncols; ++j) {
      part1 += trails[j] > 0 ? 1 : 0;
      part2 += trails[j];
    }
    memset(trails, 0, nrows * ncols * sizeof(int));
  }
  printf("Part 1: %d\n", part1);
  printf("Part 2: %d\n", part2);

  free(start);
  free(trails);
  free(grid);

  return 0;
}
