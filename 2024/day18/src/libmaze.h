#ifndef _MAZE_H_
#define _MAZE_H_

enum Direction { UP, RIGHT, DOWN, LEFT };

typedef struct {
  int x, y;
  int n_previous;
  enum Direction direction;
  int *previous;
} LastCell;

typedef struct {
  int size, n_pixel;
  int startX, startY;
  int endX, endY;
  int *wallX, *wallY;
  int *walls;
  int *visited;
} Maze;

typedef struct {
  LastCell *path;
  int priority;
} BinaryHeapNode;

typedef struct {
  int size, capacity;
  BinaryHeapNode *nodes;
} BinaryHeap;

LastCell *firstCell(int startX, int startY);
void freeCell(LastCell *cell);

BinaryHeap *createBinaryHeap();
void freeBinaryHeap(BinaryHeap *bh);
void insertHeap(BinaryHeap *bh, LastCell *path, int priority);
void deleteHeap(BinaryHeap *bh, int index);

Maze *readMaze(char *filename, int size);
void setWalls(Maze *maze, int n_select);
void resetVisited(Maze *maze);
void freeMaze(Maze *maze);
void plotMaze(Maze *maze, LastCell *path);
BinaryHeap *initializeHeap(Maze *maze);

LastCell *solveMaze(Maze *maze, BinaryHeap *moves);
int findFirstByte(Maze *maze, int start_index);

#endif
