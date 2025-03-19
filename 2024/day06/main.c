#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Direction { Up, Right, Down, Left };

const size_t MAX_LINE_SIZE = 256;
const size_t MAX_LINE = 256;

typedef struct {
  int nrows;
  int ncols;
  char **cells;
  int *visited;
  int *previous;
} Grid;

Grid *createGrid(char *obstacles, int nrows, int ncols) {
  Grid *grid = (Grid *)malloc(sizeof(Grid));
  grid->nrows = nrows;
  grid->ncols = ncols;
  grid->cells = (char **)malloc(sizeof(char *) * nrows);
  grid->cells[0] = obstacles;
  for (int i = 1; i < nrows; ++i) {
    grid->cells[i] = grid->cells[i - 1] + ncols;
  }
  grid->visited = (int *)calloc(sizeof(int), nrows * ncols);
  grid->previous = (int *)calloc(sizeof(int), nrows * ncols * 4);
  return grid;
}

void clearMemory(Grid *grid) {
  memset(grid->visited, 0, grid->nrows * grid->ncols * sizeof(int));
  memset(grid->previous, 0, grid->nrows * grid->ncols * 4 * sizeof(int));
}

void destroyGrid(Grid *grid) {
  free(grid->cells);
  free(grid->visited);
  free(grid->previous);
  free(grid);
}

int getTotalVisited(Grid *grid) {
  int nVisited = 0;
  for (int i = 0; i < grid->nrows * grid->ncols; ++i) {
    nVisited += grid->visited[i];
  }
  return nVisited;
}

int countVisited(Grid *grid, int startX, int startY, enum Direction direction) {
  clearMemory(grid);
  int x = startX;
  int y = startY;
  int nextX, nextY;
  int index;
  while (1) {
    index = 4 * (y * grid->ncols + x) + direction;
    if (grid->previous[index]) {
      return -1;
    }
    grid->previous[index] = 1;
    grid->visited[y * grid->ncols + x] = 1;
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
    if (nextX >= grid->ncols || nextX < 0 || nextY < 0 ||
        nextY >= grid->nrows) {
      break;
    }
    if (grid->cells[nextY][nextX] == '#') {
      direction = (direction + 1) % 4;
    } else {
      x = nextX;
      y = nextY;
    }
  }
  return getTotalVisited(grid);
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
  Grid *grid = createGrid(obstacles, nrows, ncols);
  int nVisited;
  nVisited = countVisited(grid, startX, startY, Up);
  printf("Part 1: %d\n", nVisited);
  int part2 = 0;
  for (int i = 0; i < nrows; ++i) {
    for (int j = 0; j < ncols; ++j) {
      if (grid->cells[i][j] == '#' || grid->cells[i][j] == '^')
        continue;
      grid->cells[i][j] = '#';
      nVisited = countVisited(grid, startX, startY, Up);
      if (nVisited < 0) {
        part2++;
      }
      grid->cells[i][j] = '.';
    }
  }
  printf("Part 2: %d\n", part2);
  free(obstacles);
  destroyGrid(grid);
  return 0;
}
