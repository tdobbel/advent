#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Direction { Up, Right, Down, Left };

const size_t MAX_LINE_SIZE = 256;
const size_t MAX_LINE = 256;

int countVisited(char **grid, int nrows, int ncols, int startX, int startY,
                 enum Direction direction) {
  int *visited = (int *)calloc(sizeof(int), nrows * ncols);
  int *previous = (int *)calloc(sizeof(int), nrows * ncols * 4);
  int blocked = 0;
  int x = startX;
  int y = startY;
  int nextX, nextY;
  int index;
  while (1) {
    index = 4 * (y * ncols + x) + direction;
    if (previous[index]) {
      blocked = 1;
      break;
    }
    previous[index] = 1;
    visited[y * ncols + x] = 1;
    nextX = x;
    nextY = y;
    switch (direction) {
    case Up:
      nextY -= 1;
      break;
    case Left:
      nextX -= 1;
      break;
    case Down:
      nextY += 1;
      break;
    case Right:
      nextX += 1;
      break;
    }
    if (nextX >= ncols || nextX < 0 || nextY < 0 || nextY >= nrows) {
      break;
    }
    if (grid[nextY][nextX] == '#') {
      direction = (direction + 1) % 4;
    } else {
      x = nextX;
      y = nextY;
    }
  }
  int nVisited = 0;
  if (blocked) {
    nVisited = -1;
  } else {
    for (int i = 0; i < nrows * ncols; ++i) {
      nVisited += visited[i];
    }
  }
  free(previous);
  free(visited);
  return nVisited;
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
  int startX, startY;
  char *obstacles = (char *)malloc(sizeof(char) * MAX_LINE_SIZE * MAX_LINE);
  while (fgets(line, MAX_LINE_SIZE, file)) {
    ncols = strcspn(line, "\n");
    for (int i = 0; i < ncols; ++i) {
      obstacles[index++] = line[i];
      if (line[i] == '^') {
        startX = i;
        startY = nrows;
      }
    }
    nrows++;
  }
  fclose(file);

  obstacles = (char *)realloc(obstacles, sizeof(char) * index);
  // Re-arrange obstacles as 2D grid
  char **grid = (char **)malloc(sizeof(char *) * nrows);
  grid[0] = obstacles;
  for (int i = 1; i < nrows; ++i) {
    grid[i] = grid[i - 1] + ncols;
  }
  int nVisited;
  nVisited = countVisited(grid, nrows, ncols, startX, startY, Up);
  printf("Part 1: %d\n", nVisited);
  int part2 = 0;
  for (int i = 0; i < nrows; ++i) {
    for (int j = 0; j < ncols; ++j) {
      if (grid[i][j] == '#' || grid[i][j] == '^')
        continue;
      grid[i][j] = '#';
      nVisited = countVisited(grid, nrows, ncols, startX, startY, Up);
      if (nVisited < 0) {
        part2++;
      }
      grid[i][j] = '.';
    }
  }
  printf("Part 2: %d\n", part2);

  free(obstacles);
  free(grid);
  return 0;
}
