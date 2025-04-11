#include "libmaze.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256

Maze *readMaze(char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Failed to open file");
    return NULL;
  }
  char buffer[BUFFER_SIZE];
  int ny = 0, nx;
  Maze *maze = malloc(sizeof(Maze));
  maze->walls = (int *)calloc(sizeof(int), BUFFER_SIZE * BUFFER_SIZE);
  while (fgets(buffer, sizeof(buffer), file)) {
    nx = strcspn(buffer, "\n");
    for (int i = 0; i < nx; ++i) {
      maze->walls[ny * nx + i] = buffer[i] == '#' ? 1 : 0;
      if (buffer[i] == 'S') {
        maze->startX = i;
        maze->startY = ny;
      } else if (buffer[i] == 'E') {
        maze->endX = i;
        maze->endY = ny;
      }
    }
    ny++;
  }
  fclose(file);

  maze->walls = realloc(maze->walls, sizeof(int) * nx * ny);
  maze->nx = nx;
  maze->ny = ny;
  maze->visited = (int *)calloc(sizeof(int), nx * ny);

  return maze;
}

void plotMaze(Maze *maze) {
  for (int y = 0; y < maze->ny; ++y) {
    for (int x = 0; x < maze->nx; ++x) {
      if (maze->walls[y * maze->nx + x] == 1) {
        printf("#");
      } else if (x == maze->startX && y == maze->startY) {
        printf("S");
      } else if (x == maze->endX && y == maze->endY) {
        printf("E");
      } else {
        printf(".");
      }
    }
    printf("\n");
  }
}

void resetVisited(Maze *maze) {
  memset(maze->visited, 0, sizeof(int) * maze->nx * maze->ny);
}

void freeMaze(Maze *maze) {
  if (maze) {
    free(maze->walls);
    free(maze->visited);
    free(maze);
  }
}

LastCell *firstCell(int startX, int startY) {
  LastCell *cell = malloc(sizeof(LastCell));
  cell->x = startX;
  cell->y = startY;
  cell->score = 0;
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

struct Node *createNode(LastCell *cell) {
  struct Node *node = malloc(sizeof(struct Node));
  node->cell = cell;
  node->next = NULL;
  return node;
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

static int computePriority(Maze *maze, LastCell *cell) {
  return cell->score + (maze->endX - cell->x) + (cell->y - maze->endY);
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
    int index = y * maze->nx + x;
    if (maze->walls[index] == 1 || maze->visited[index] == 1) {
      continue;
    }
    LastCell *newCell = malloc(sizeof(LastCell));
    newCell->x = x;
    newCell->y = y;
    newCell->score = cell->score + (i == 0 ? 1 : 1001);
    newCell->n_previous = cell->n_previous + 1;
    newCell->direction = directions[i];
    newCell->previous = (int *)malloc(sizeof(int) * (cell->n_previous + 1));
    if (cell->previous != NULL)
      memcpy(newCell->previous, cell->previous, sizeof(int) * cell->n_previous);
    newCell->previous[cell->n_previous] = cell->x + cell->y * maze->nx;
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

int distance(int x0, int y0, int x1, int y1) {
  return abs(x0 - x1) + abs(y0 - y1);
}

int countShortcuts(Maze *maze, LastCell *path, int minSave, int maxCheat) {
  int *pathX = malloc(sizeof(int) * (path->n_previous + 1));
  int *pathY = malloc(sizeof(int) * (path->n_previous + 1));
  for (int i = 0; i < path->n_previous; ++i) {
    pathX[i] = path->previous[i] % maze->nx;
    pathY[i] = path->previous[i] / maze->nx;
  }
  pathX[path->n_previous] = maze->endX;
  pathY[path->n_previous] = maze->endY;
  int result = 0;
  for (int i = 0; i < path->n_previous + 1 - minSave; ++i) {
    for (int j = i + minSave; j < path->n_previous + 1; ++j) {
      int dist = distance(pathX[i], pathY[i], pathX[j], pathY[j]);
      if (dist > maxCheat)
        continue;
      int save = j - i - dist;
      if (save >= minSave) {
        result++;
      }
    }
  }
  free(pathX);
  free(pathY);
  return result;
}
