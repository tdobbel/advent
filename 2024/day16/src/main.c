#include "maze.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  Maze *maze = readMaze(argv[1]);
  if (!maze) {
    fprintf(stderr, "Failed to parse maze\n");
    return EXIT_FAILURE;
  }

  // Part 1;
  LastCell *start = firstCell(maze->startX, maze->startY);
  LinkedList *moves = initList();
  push(moves, start);

  LastCell *end = solveMaze(maze, moves);
  if (end) {
    printf("%d\n", end->score);
  }

  freeCell(end);
  freeMaze(maze);
  freeList(moves);

  return EXIT_SUCCESS;
}
