#ifndef DAY15_H
#define DAY15_H

#include "array_list.h"

enum ObjectType { BOX, WALL };

typedef struct {
  enum ObjectType type;
  int x, y;
} ObjectBase;

typedef struct {
  enum ObjectType type;
  int x, y, width;
} Object;

typedef struct {
  int **grid;
  int nx, ny, nObject;
  Object *objects;
} Warehouse;

Warehouse *createWarehouse(ObjectBase *objectPositions, int nx, int ny,
                           int nObject, int width);
void updateObjectPositions(Warehouse *self, ArrayList *imoved, int dx, int dy);
void freeWarehouse(Warehouse *self);
int solvePuzzle(ObjectBase *objectPositions, int nObj, int nx, int ny,
                int startX, int startY, ArrayList *moves, int width);
void moveRobot(Warehouse *self, char move, int *robotX, int *robotY);
void plotGrid(Warehouse *self, int robotX, int robotY);

#endif
