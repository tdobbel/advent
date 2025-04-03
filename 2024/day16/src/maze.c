#include "maze.h"
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
  if (cell->previous)
    free(cell->previous);
  free(cell);
}

struct Node *createNode(LastCell *cell) {
  struct Node *node = malloc(sizeof(struct Node));
  node->cell = cell;
  node->next = NULL;
  return node;
}

LinkedList *initList() {
  LinkedList *list = malloc(sizeof(LinkedList));
  list->size = 0;
  list->head = NULL;
  list->tail = NULL;
  return list;
}

void push(LinkedList *list, LastCell *cell) {
  struct Node *node = createNode(cell);
  if (list->size == 0) {
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node;
    list->tail = node;
  }
  list->size++;
}

LastCell *get(LinkedList *list, int index) {
  if (index < 0 || index >= list->size) {
    return NULL;
  }
  struct Node *node = list->head;
  for (int i = 0; i < index; ++i) {
    node = node->next;
  }
  return node->cell;
}

LastCell *pop(LinkedList *list, int index) {
  if (index < 0 || index >= list->size) {
    return NULL;
  }
  struct Node *prev = NULL;
  struct Node *node = list->head;
  for (int i = 0; i < index; ++i) {
    prev = node;
    node = prev->next;
  }
  LastCell *cell = node->cell;
  if (prev == NULL) {
    list->head = node->next;
  } else {
    prev->next = node->next;
  }
  if (node->next == NULL)
    list->tail = prev;
  node->next = NULL;
  free(node);
  list->size--;
  return cell;
}

void emptyList(LinkedList *list) {
  struct Node *node = list->head;
  while (node) {
    struct Node *next = node->next;
    freeCell(node->cell);
    free(node);
    node = next;
  }
  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
}

void freeList(LinkedList *list) {
  struct Node *node = list->head;
  while (node) {
    struct Node *next = node->next;
    freeCell(node->cell);
    free(node);
    node = next;
  }
  free(list);
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
void nextMoves(Maze *maze, LastCell *cell, LinkedList *moves) {
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
    push(moves, newCell);
    maze->visited[index] = 1;
  }
}

LastCell *solveMaze(Maze *maze, LinkedList *moves) {
  struct Node *node;
  LastCell *cell = NULL;
  while (moves->size > 0) {
    int best = INT_MAX;
    int i = 0, ibest = 0;
    node = moves->head;
    while (node) {
      cell = node->cell;
      int score = cell->score + (maze->endX - cell->x) + (cell->y - maze->endY);
      if (score < best) {
        best = score;
        ibest = i;
      }
      node = node->next;
      i++;
    }
    cell = pop(moves, ibest);
    if (cell->x == maze->endX && cell->y == maze->endY) {
      emptyList(moves);
      return cell;
    }
    nextMoves(maze, cell, moves);
    freeCell(cell);
  }

  return NULL;
}

int findAllPaths(Maze *maze, LastCell *ref) {
  LastCell *start = firstCell(maze->startX, maze->startY);
  int *bestPathCell = calloc(sizeof(int), maze->nx * maze->ny);
  for (int i = 0; i < ref->n_previous; ++i) {
    bestPathCell[ref->previous[i]] = 1;
  }
  bestPathCell[maze->endX + maze->endY * maze->nx] = 1;
  LinkedList *moves = initList();
  LastCell *next, *end;
  for (int i = 1; i < ref->n_previous - 1; ++i) {
    resetVisited(maze);
    nextMoves(maze, start, moves);
    freeCell(start);
    // Check possible next moves
    for (int j = 0; j < moves->size; ++j) {
      next = get(moves, j);
      if (next->x + next->y * maze->nx == ref->previous[i]) {
        start = pop(moves, j);
        break;
      }
    }
    // No alternative path possible
    if (moves->size == 0)
      continue;
    maze->visited[start->x + start->y * maze->nx] = 1;
    end = solveMaze(maze, moves);
    if (end && end->score <= ref->score) {
      for (int j = 0; j < end->n_previous; ++j) {
        bestPathCell[end->previous[j]] = 1;
      }
    }
  }
  int result = 0;
  for (int i = 0; i < maze->ny * maze->ny; ++i) {
    result += bestPathCell[i];
  }
  freeList(moves);
  freeCell(start);
  freeCell(end);
  free(bestPathCell);
  return result;
}
