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
  if (!end) {
    fprintf(stderr, "No solution found in part 1 :(\n");
    return EXIT_FAILURE;
  }
  printf("Part 1: %d\n", end->score);

  // Part 2;
  printf("Part 2: %d\n", findAllPaths(maze, end));

  freeCell(end);
  freeMaze(maze);
  freeList(moves);

  return EXIT_SUCCESS;
}
