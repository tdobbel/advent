#include "day15.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void parseMove(const char c, int *dx, int *dy) {
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

Warehouse *createWarehouse(ObjectBase *objectPositions, int nx, int ny,
                           int nObject, int width) {
  int *content = (int *)malloc(nx * width * ny * sizeof(int));
  memset(content, -1, nx * width * ny * sizeof(int));
  Warehouse *self = (Warehouse *)malloc(sizeof(Warehouse));
  self->grid = (int **)malloc(ny * sizeof(int *));
  self->nx = nx;
  self->ny = ny;
  self->nObject = nObject;
  self->grid[0] = content;
  for (int i = 1; i < ny; ++i) {
    self->grid[i] = self->grid[i - 1] + nx * width;
  }
  self->objects = malloc(nObject * sizeof(Object));
  int iw;
  for (int iobj = 0; iobj < nObject; ++iobj) {
    ObjectBase *base = objectPositions + iobj;
    Object *obj = self->objects + iobj;
    obj->x = base->x * width;
    obj->y = base->y;
    obj->type = base->type;
    obj->width = width;
    for (iw = 0; iw < width; ++iw) {
      self->grid[obj->y][obj->x + iw] = iobj;
    }
  }
  return self;
}

void freeWarehouse(Warehouse *self) {
  free(self->grid[0]);
  free(self->grid);
  free(self->objects);
  free(self);
}

void updateObjectPositions(Warehouse *self, ArrayList *imoved, int dx, int dy) {
  int i, iobj, iw;
  for (i = 0; i < imoved->size; ++i) {
    iobj = getInt(imoved, i);
    Object *obj = self->objects + iobj;
    for (iw = 0; iw < obj->width; ++iw) {
      self->grid[obj->y][obj->x + iw] = -1;
    }
    obj->x += dx;
    obj->y += dy;
  }
  for (i = 0; i < imoved->size; ++i) {
    iobj = getInt(imoved, i);
    Object *obj = self->objects + iobj;
    for (iw = 0; iw < obj->width; ++iw) {
      self->grid[obj->y][obj->x + iw] = iobj;
    }
  }
}

int summedGpsScore(Warehouse *self) {
  int score = 0;
  for (int i = 0; i < self->nObject; ++i) {
    Object *o = self->objects + i;
    if (o->type == BOX) {
      score += 100 * o->y + o->x;
    }
  }
  return score;
}

void plotGrid(Warehouse *self, int robotX, int robotY) {
  for (int y = 0; y < self->ny; ++y) {
    for (int x = 0; x < self->nx; ++x) {
      if (x == robotX && y == robotY) {
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

void moveRobot(Warehouse *self, char move, int *robotX, int *robotY) {
  int dx, dy;
  parseMove(move, &dx, &dy);
  int x = *robotX + dx;
  int y = *robotY + dy;
  if (self->grid[y][x] < 0) {
    *robotX = x;
    *robotY = y;
    return;
  }
  int blocked = 0;
  ArrayList *queue = createArrayList(DT_INT, 16);
  ArrayList *imoved = createArrayList(DT_INT, 16);
  appendInt(queue, self->grid[y][x]);
  int iobj;
  while (queue->size > 0) {
    iobj = popInt(queue);
    Object *obj = self->objects + iobj;
    if (obj->type == WALL) {
      blocked = 1;
      break;
    }
    if (containsInt(imoved, iobj)) {
      continue;
    }
    appendInt(imoved, iobj);
    if (dx == 0) {
      for (int iw = 0; iw < obj->width; ++iw) {
        iobj = self->grid[obj->y + dy][obj->x + iw];
        if (iobj >= 0 && !containsInt(imoved, iobj)) {
          appendInt(queue, iobj);
        }
      }
    } else if (dx < 0) {
      iobj = self->grid[obj->y][obj->x - 1];
      if (iobj >= 0 && !containsInt(imoved, iobj)) {
        appendInt(queue, iobj);
      }
    } else {
      iobj = self->grid[obj->y][obj->x + obj->width];
      if (iobj >= 0 && !containsInt(imoved, iobj)) {
        appendInt(queue, iobj);
      }
    }
  }

  freeArrayList(queue);

  if (!blocked) {
    updateObjectPositions(self, imoved, dx, dy);
    *robotX += dx;
    *robotY += dy;
  }
  freeArrayList(imoved);
}

int solvePuzzle(ObjectBase *objectPositions, int nObj, int nx, int ny,
                int startX, int startY, ArrayList *moves, int width) {
  Warehouse *wh = createWarehouse(objectPositions, nx * width, ny, nObj, width);
  int robotX = startX * width;
  int robotY = startY;
  // plotGrid(wh, robotX, robotY);
  for (int i = 0; i < moves->size; ++i) {
    // printf("Move: %c\n", getChar(moves, i));
    moveRobot(wh, getChar(moves, i), &robotX, &robotY);
    // plotGrid(wh, robotX, robotY);
  }
  int score = summedGpsScore(wh);
  freeWarehouse(wh);
  return score;
}
