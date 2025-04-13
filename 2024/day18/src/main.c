#include "libmaze.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  int n_pixel, size;
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "input") == 0) {
    n_pixel = 1024;
    size = 71;
  } else if (strcmp(argv[1], "example") == 0) {
    n_pixel = 12;
    size = 7;
  } else {
    fprintf(stderr, "Unexpected file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  Maze *maze = readMaze(argv[1], size);
  BinaryHeap *moves;
  LastCell *path;
  setWalls(maze, n_pixel);
  moves = initializeHeap(maze);
  path = solveMaze(maze, moves);
  freeBinaryHeap(moves);
  if (path) {
    printf("Path found in %d steps\n", path->n_previous);
    // plotMaze(maze, path);
    freeCell(path);
  }

  int stop = findFirstByte(maze, n_pixel);
  printf("Maze becomes unsolveable with pixel %d,%d\n", maze->wallX[stop],
         maze->wallY[stop]);

  freeMaze(maze);

  return EXIT_SUCCESS;
}
