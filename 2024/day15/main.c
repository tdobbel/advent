#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define MAX_OBJECT 2500

enum ObjectType { BOX, WALL };

typedef struct {
  enum ObjectType type;
  int x, y, width;
} Object;

typedef struct {
  int **grid;
  int nx, ny, nObject;
  Object *objects;
} Warehouse;

void getVector(const char c, int *dx, int *dy) {
  *dx = 0;
  *dy = 0;
  switch (c) {
  case '^':
    *dy = -1;
    return;
  case 'v':
    *dy = 1;
    return;
  case '<':
    *dx = -1;
    return;
  case '>':
    *dx = 1;
    return;
  }
}

Warehouse *createWarehouse(int nx, int ny, Object *objects, int nObject) {
  int *content = (int *)malloc(nx * ny * sizeof(int));
  memset(content, -1, nx * ny * sizeof(int));
  Warehouse *self = (Warehouse *)malloc(sizeof(Warehouse));
  self->grid = (int **)malloc(ny * sizeof(int *));
  self->nx = nx;
  self->ny = ny;
  self->nObject = nObject;
  self->grid[0] = content;
  for (int i = 1; i < ny; ++i) {
    self->grid[i] = self->grid[i - 1] + nx;
  }
  for (int i = 0; i < nObject; ++i) {
    Object *o = objects + i;
    for (int j = 0; j < o->width; ++j) {
      self->grid[o->y][o->x + j] = i;
    }
  }
  self->objects = objects;
  return self;
}

void freeWarehouse(Warehouse *self) {
  free(self->grid[0]);
  free(self->grid);
  free(self->objects);
  free(self);
}

void moveObjects(Warehouse *self, int *objectIds, int nMoved, int dx, int dy) {
  Object *o;
  for (int i = 0; i < nMoved; ++i) {
    o = self->objects + objectIds[i];
    for (int j = 0; j < o->width; ++j) {
      self->grid[o->y][o->x + j] = -1;
    }
  }
  for (int i = 0; i < nMoved; ++i) {
    o = self->objects + objectIds[i];
    o->x += dx;
    o->y += dy;
    for (int j = 0; j < o->width; ++j) {
      self->grid[o->y][o->x + j] = objectIds[i];
    }
  }
}

void getMovedObjects(Warehouse *self, int x, int y, int dx, int dy,
                     int *movingIds, int *nMoving, int *blocked) {
  int x_ = x + dx;
  int y_ = y + dy;
  if (self->grid[y_][x_] < 0)
    return;
  int iobj = self->grid[y_][x_];
  for (int i = 0; i < *nMoving; ++i) {
    if (movingIds[i] == iobj)
      return;
  }
  Object *o = self->objects + iobj;
  if (o->type == WALL) {
    *blocked = 1;
    return;
  }
  movingIds[(*nMoving)++] = iobj;
  if (dy == 0) {
    getMovedObjects(self, o->width == 2 ? x_ + dx : x_, y_, dx, dy, movingIds,
                    nMoving, blocked);
  } else {

    for (int j = 0; j < o->width; ++j) {
      getMovedObjects(self, o->x + j, y_, dx, dy, movingIds, nMoving, blocked);
    }
  }
}

void moveRobot(Warehouse *self, int *x, int *y, int dx, int dy) {
  int cntr = 0;
  int x_ = *x + dx;
  int y_ = *y + dy;
  int objectids[100];
  int blocked = 0;
  getMovedObjects(self, *x, *y, dx, dy, objectids, &cntr, &blocked);
  if (blocked)
    return;
  moveObjects(self, objectids, cntr, dx, dy);
  *x += dx;
  *y += dy;
}

int gpsScore(Warehouse *self) {
  int score = 0;
  for (int i = 0; i < self->nObject; ++i) {
    Object *o = self->objects + i;
    if (o->type == BOX) {
      score += 100 * o->y + o->x;
    }
  }
  return score;
}

void plotGrid(Warehouse *self, int robot[2]) {
  for (int y = 0; y < self->ny; ++y) {
    for (int x = 0; x < self->nx; ++x) {
      if (robot[0] == x && robot[1] == y) {
        printf("@");
        continue;
      }
      int iobj = self->grid[y][x];
      if (iobj < 0) {
        printf(".");
        continue;
      }
      Object *o = self->objects + iobj;
      if (o->type == WALL) {
        printf("#");
      } else if (o->width == 1) {
        printf("O");
      } else {
        printf(x == o->x ? "[" : "]");
      }
    }
    printf("\n");
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf(("Missing input file\n"));
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    printf("Error opening file\n");
    return 1;
  }

  int nx, ny = 0;
  int startX, startY;
  int nObject = 0;
  Object *objects = (Object *)malloc(MAX_OBJECT * sizeof(Object));

  char buffer[BUFFER_SIZE];
  while (fgets(buffer, BUFFER_SIZE, file)) {
    int n = strcspn(buffer, "\n");
    if (n == 0)
      break;
    nx = n;
    for (int x = 0; x < nx; ++x) {
      if (buffer[x] == '.')
        continue;
      if (buffer[x] == '@') {
        startX = x;
        startY = ny;
        continue;
      }
      enum ObjectType type = buffer[x] == '#' ? WALL : BOX;
      if (nObject == MAX_OBJECT) {
        printf("Too many objects\n");
        return 1;
      }
      Object *o = &objects[nObject++];
      o->type = type;
      o->x = x;
      o->y = ny;
      o->width = 1;
    }
    ny++;
  }

  int robot[2] = {startX, startY};
  int robot2[2] = {startX * 2, startY};

  objects = (Object *)realloc(objects, nObject * sizeof(Object));
  Object *objects2 = (Object *)malloc(nObject * sizeof(Object));
  for (int i = 0; i < nObject; ++i) {
    Object *ref = objects + i;
    Object *o = objects2 + i;
    o->type = ref->type;
    o->x = ref->x * 2;
    o->y = ref->y;
    o->width = 2;
  }

  Warehouse *wh = createWarehouse(nx, ny, objects, nObject);
  Warehouse *wh2 = createWarehouse(nx * 2, ny, objects2, nObject);

  int dx, dy;
  while (fgets(buffer, BUFFER_SIZE, file)) {
    int n = strcspn(buffer, "\n");
    for (int i = 0; i < n; ++i) {
      getVector(buffer[i], &dx, &dy);
      moveRobot(wh, robot + 0, robot + 1, dx, dy);
      moveRobot(wh2, robot2 + 0, robot2 + 1, dx, dy);
    }
  }
  fclose(file);

  int part1 = gpsScore(wh);
  printf("Part 1: %d\n", part1);
  int part2 = gpsScore(wh2);
  printf("Part 2: %d\n", part2);

  freeWarehouse(wh);
  freeWarehouse(wh2);

  return 0;
}
