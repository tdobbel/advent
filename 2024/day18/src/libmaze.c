#include "libmaze.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256

Maze *readMaze(char *filename, int size) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Failed to open file");
    return NULL;
  }
  char buffer[BUFFER_SIZE];
  Maze *maze = malloc(sizeof(Maze));
  maze->size = size;
  maze->startX = 0;
  maze->startY = 0;
  maze->endX = size - 1;
  maze->endY = size - 1;
  maze->walls = calloc(sizeof(int), size * size);
  int capacity = 2048;
  maze->wallX = malloc(sizeof(int) * capacity);
  maze->wallY = malloc(sizeof(int) * capacity);
  int n_pixel = 0;
  int x, y;
  while (fgets(buffer, sizeof(buffer), file)) {
    sscanf(buffer, "%d,%d", &x, &y);
    if (n_pixel == capacity) {
      capacity *= 2;
      maze->wallX = realloc(maze->wallX, sizeof(int) * capacity);
      maze->wallY = realloc(maze->wallY, sizeof(int) * capacity);
    }
    maze->wallX[n_pixel] = x;
    maze->wallY[n_pixel] = y;
    n_pixel++;
  }
  fclose(file);
  maze->n_pixel = n_pixel;
  maze->wallX = realloc(maze->wallX, sizeof(int) * n_pixel);
  maze->wallY = realloc(maze->wallY, sizeof(int) * n_pixel);
  maze->visited = (int *)calloc(sizeof(int), size * size);

  return maze;
}

void plotMaze(Maze *maze, LastCell *path) {
  char *output = (char *)malloc(sizeof(char) * maze->size * maze->size);
  for (int y = 0; y < maze->size; ++y) {
    for (int x = 0; x < maze->size; ++x) {
      int index = y * maze->size + x;
      if (maze->walls[y * maze->size + x] == 1) {
        output[index] = '#';
      } else if (x == maze->startX && y == maze->startY) {
        output[index] = 'S';
      } else if (x == maze->endX && y == maze->endY) {
        output[index] = 'E';
      } else {
        output[index] = '.';
      }
    }
  }
  if (path) {
    output[path->y * maze->size + path->x] = 'O';
    for (int i = 0; i < path->n_previous; ++i) {
      int index = path->previous[i];
      output[index] = 'O';
    }
  }
  for (int y = 0; y < maze->size; ++y) {
    for (int x = 0; x < maze->size; ++x) {
      printf("%c", output[y * maze->size + x]);
    }
    printf("\n");
  }
  free(output);
}

void resetVisited(Maze *maze) {
  memset(maze->visited, 0, sizeof(int) * maze->size * maze->size);
}

void setWalls(Maze *maze, int n_select) {
  memset(maze->walls, 0, sizeof(int) * maze->size * maze->size);
  for (int i = 0; i < n_select; ++i) {
    int x = maze->wallX[i];
    int y = maze->wallY[i];
    maze->walls[y * maze->size + x] = 1;
  }
}

void freeMaze(Maze *maze) {
  if (maze) {
    free(maze->walls);
    free(maze->visited);
    free(maze->wallX);
    free(maze->wallY);
    free(maze);
  }
}

LastCell *firstCell(int startX, int startY) {
  LastCell *cell = malloc(sizeof(LastCell));
  cell->x = startX;
  cell->y = startY;
  cell->n_previous = 0;
  cell->direction = RIGHT;
  cell->previous = NULL;
  return cell;
}

void freeCell(LastCell *cell) {
  if (cell->previous != NULL)
    free(cell->previous);
  free(cell);
}

BinaryHeap *createBinaryHeap() {
  BinaryHeap *bh = malloc(sizeof(BinaryHeap));
  bh->size = 0;
  bh->capacity = 15;
  bh->nodes = malloc(sizeof(BinaryHeapNode) * bh->capacity);
  return bh;
}

void freeBinaryHeap(BinaryHeap *bh) {
  for (int i = 0; i < bh->size; ++i) {
    BinaryHeapNode *node = bh->nodes + i;
    freeCell(node->path);
  }
  free(bh->nodes);
  free(bh);
}

void swapHeapNodes(BinaryHeapNode *a, BinaryHeapNode *b) {
  BinaryHeapNode temp = *a;
  *a = *b;
  *b = temp;
}

static void heapify(BinaryHeap *bh, int index) {
  int left = 2 * index + 1;
  int right = 2 * index + 2;
  int smallest = index;
  if (left < bh->size &&
      bh->nodes[left].priority < bh->nodes[smallest].priority) {
    smallest = left;
  }
  if (right < bh->size &&
      bh->nodes[right].priority < bh->nodes[smallest].priority) {
    smallest = right;
  }
  if (smallest != index) {
    swapHeapNodes(bh->nodes + index, bh->nodes + smallest);
    heapify(bh, smallest);
  }
}

void deleteHeap(BinaryHeap *bh, int index) {
  if (index < 0 || index >= bh->size) {
    return;
  }
  bh->nodes[index] = bh->nodes[bh->size - 1];
  bh->size--;
  heapify(bh, index);
}

LastCell *popHeap(BinaryHeap *bh) {
  if (bh->size == 0)
    return NULL;
  LastCell *path = bh->nodes[0].path;
  deleteHeap(bh, 0);
  return path;
}

void insertHeap(BinaryHeap *bh, LastCell *path, int priority) {
  if (bh->size == bh->capacity) {
    bh->capacity *= 2;
    bh->nodes = realloc(bh->nodes, sizeof(BinaryHeapNode) * bh->capacity);
  }
  BinaryHeapNode *node = bh->nodes + bh->size;
  node->path = path;
  node->priority = priority;
  bh->size++;
  int i = bh->size - 1;
  while (i > 0) {
    if (bh->nodes[(i - 1) / 2].priority <= bh->nodes[i].priority)
      break;
    swapHeapNodes(bh->nodes + i, bh->nodes + (i - 1) / 2);
    i = (i - 1) / 2;
  }
}

void dirdx(enum Direction dir, int *dx, int *dy) {
  switch (dir) {
  case UP:
    *dx = 0;
    *dy = -1;
    break;
  case RIGHT:
    *dx = 1;
    *dy = 0;
    break;
  case DOWN:
    *dx = 0;
    *dy = 1;
    break;
  case LEFT:
    *dx = -1;
    *dy = 0;
    break;
  }
}

static int distance(int x0, int y0, int x1, int y1) {
  return abs(x0 - x1) + abs(y0 - y1);
}

static int computePriority(Maze *maze, LastCell *cell) {
  return cell->n_previous;
}

BinaryHeap *initializeHeap(Maze *maze) {
  BinaryHeap *bh = createBinaryHeap();
  LastCell *start = firstCell(maze->startX, maze->startY);
  int priority = computePriority(maze, start);
  insertHeap(bh, start, priority);
  return bh;
}

void nextMoves(Maze *maze, LastCell *cell, BinaryHeap *moves) {
  int directions[3] = {cell->direction, (cell->direction + 1) % 4,
                       (cell->direction + 3) % 4};
  int dx, dy;
  for (int i = 0; i < 3; ++i) {
    dirdx(directions[i], &dx, &dy);
    int x = cell->x + dx;
    int y = cell->y + dy;
    if (x < 0 || x >= maze->size || y < 0 || y >= maze->size) {
      continue;
    }
    int index = y * maze->size + x;
    if (maze->walls[index] == 1 || maze->visited[index] == 1) {
      continue;
    }
    LastCell *newCell = malloc(sizeof(LastCell));
    newCell->x = x;
    newCell->y = y;
    newCell->n_previous = cell->n_previous + 1;
    newCell->direction = directions[i];
    newCell->previous = (int *)malloc(sizeof(int) * (cell->n_previous + 1));
    if (cell->previous != NULL)
      memcpy(newCell->previous, cell->previous, sizeof(int) * cell->n_previous);
    newCell->previous[cell->n_previous] = cell->x + cell->y * maze->size;
    int priority = computePriority(maze, newCell);
    insertHeap(moves, newCell, priority);
    maze->visited[index] = 1;
  }
  freeCell(cell);
}

LastCell *solveMaze(Maze *maze, BinaryHeap *moves) {
  struct Node *node;
  LastCell *cell = NULL;
  while (moves->size > 0) {
    cell = popHeap(moves);
    if (cell->x == maze->endX && cell->y == maze->endY) {
      return cell;
    }
    nextMoves(maze, cell, moves);
  }

  return NULL;
}

int findFirstByte(Maze *maze, int start_index) {
  int start = start_index;
  int stop = maze->n_pixel;

  BinaryHeap *moves;
  LastCell *path;
  while (stop - start > 1) {
    int mid = (start + stop) / 2;
    setWalls(maze, mid + 1);
    resetVisited(maze);
    moves = initializeHeap(maze);
    path = solveMaze(maze, moves);
    if (path) {
      start = mid + 1;
      freeCell(path);
    } else {
      stop = mid;
    }
    freeBinaryHeap(moves);
  }
  return stop;
}
