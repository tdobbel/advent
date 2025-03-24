#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 150
#define MAX_GALAXY 1024
#define MAX_ROWS 200

uint64_t sumDistances(int *galaxies, int nGalaxy, int *galaxyRows,
                      int *galaxyCols, int ncols, uint64_t expansion) {
  uint64_t sum = 0;
  int x1, y1, x2, y2;
  int x_from, y_from, x_to, y_to;
  int i, j, k;
  for (i = 0; i < nGalaxy - 1; ++i) {
    x1 = galaxies[i] % ncols;
    y1 = galaxies[i] / ncols;
    for (j = i + 1; j < nGalaxy; ++j) {
      x2 = galaxies[j] % ncols;
      y2 = galaxies[j] / ncols;
      if (x2 > x1) {
        x_from = x1;
        x_to = x2;
      } else {
        x_from = x2;
        x_to = x1;
      }
      if (y2 > y1) {
        y_from = y1;
        y_to = y2;
      } else {
        y_from = y2;
        y_to = y1;
      }
      uint64_t distance = (uint64_t)(y_to - y_from + x_to - x_from);
      for (k = y_from + 1; k < y_to; ++k) {
        if (!galaxyRows[k])
          distance += expansion - 1;
      }
      for (k = x_from + 1; k < x_to; ++k) {
        if (!galaxyCols[k])
          distance += expansion - 1;
      }
      sum += distance;
    }
  }
  return sum;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    perror("Missing input file\n");
    return -1;
  }

  FILE *file = fopen(argv[1], "r");
  char buffer[BUFFER_SIZE];
  int *galaxies = (int *)malloc(MAX_GALAXY * sizeof(int));
  int cntr = 0;
  int nrows = 0, ncols;
  int *galaxyCols;
  int *galaxyRows = (int *)calloc(sizeof(int), MAX_ROWS);
  while (fgets(buffer, BUFFER_SIZE, file)) {
    if (nrows >= MAX_ROWS) {
      perror("Too many rows !\n");
      return -1;
    }
    ncols = strcspn(buffer, "\n");
    if (nrows == 0) {
      galaxyCols = (int *)calloc(sizeof(int), ncols);
    }
    for (int i = 0; i < ncols; ++i) {
      if (buffer[i] == '#') {
        if (cntr >= MAX_GALAXY) {
          perror("Too many galaxies !\n");
          return -1;
        }
        galaxies[cntr++] = nrows * ncols + i;
        galaxyRows[nrows] = 1;
        galaxyCols[i] = 1;
      }
    }
    nrows++;
  }
  fclose(file);

  galaxies = (int *)realloc(galaxies, cntr * sizeof(int));
  galaxyRows = (int *)realloc(galaxyRows, nrows * sizeof(int));

  uint64_t part1 =
      sumDistances(galaxies, cntr, galaxyRows, galaxyCols, ncols, 2);
  uint64_t part2 =
      sumDistances(galaxies, cntr, galaxyRows, galaxyCols, ncols, 1000000);

  printf("Part 1: %lu\n", part1);
  printf("Part 2: %lu\n", part2);

  free(galaxies);
  free(galaxyCols);
  free(galaxyRows);
  return 0;
}
