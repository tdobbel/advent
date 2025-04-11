#include "libmaze.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  Maze *puzzle = readMaze(argv[1]);
  BinaryHeap *moves = initializeHeap(puzzle);
  LastCell *path = solveMaze(puzzle, moves);
  if (path) {
    printf("Maze solved in %d picoseconds\n", path->n_previous);
  }

  int part1 = countShortcuts(puzzle, path, 100, 2);
  printf("Part 1: %d cheats save at least 100 picoseconds\n", part1);
  int part2 = countShortcuts(puzzle, path, 100, 20);
  printf("Part 2: %d cheats save at least 100 picoseconds\n", part2);

  freeBinaryHeap(moves);
  freeMaze(puzzle);
  freeCell(path);

  return EXIT_SUCCESS;
}
