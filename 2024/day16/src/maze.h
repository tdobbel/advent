#ifndef _MAZE_H_
#define _MAZE_H_

enum Direction { UP, RIGHT, DOWN, LEFT };

typedef struct {
  int x, y;
  int score;
  int n_previous;
  enum Direction direction;
  int *previous;
} LastCell;

typedef struct {
  int nx, ny;
  int startX, startY;
  int endX, endY;
  int *walls;
  int *visited;
} Maze;

struct Node {
  LastCell *cell;
  struct Node *next;
};

typedef struct LinkedList {
  int size;
  struct Node *head;
  struct Node *tail;
} LinkedList;

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

struct Node *createNode(LastCell *cell);

LinkedList *initList();
LastCell *pop(LinkedList *list, int index);
LastCell *get(LinkedList *list, int index);
void push(LinkedList *list, LastCell *cell);
void emptyList(LinkedList *list);
void freeList(LinkedList *list);

BinaryHeap *createBinaryHeap();
void freeBinaryHeap(BinaryHeap *bh);
void insertHeap(BinaryHeap *bh, LastCell *path, int priority);
void deleteHeap(BinaryHeap *bh, int index);

Maze *readMaze(char *filename);
void resetVisited(Maze *maze);
void freeMaze(Maze *maze);
void plotMaze(Maze *maze);
BinaryHeap *initializeHeap(Maze *maze);

LastCell *solveMaze(Maze *maze, BinaryHeap *moves);
int findAllPaths(Maze *maze, LastCell *ref);

#endif
