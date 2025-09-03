#include "array_list.h"
#include "day15.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define MAX_ARRAY_SIZE 2500

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf(("Missing input file\n"));
    return EXIT_FAILURE;
  }

  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    printf("Error opening file\n");
    return EXIT_FAILURE;
  }

  int nx, ny = 0;
  int startX, startY;
  int nObject = 0;
  ObjectBase *objectPositions =
      (ObjectBase *)malloc(MAX_ARRAY_SIZE * sizeof(ObjectBase));
  ArrayList *moves = createArrayList(DT_CHAR, 128);
  int parseGrid = 1;

  char buffer[BUFFER_SIZE];
  while (fgets(buffer, BUFFER_SIZE, file)) {
    int n = strcspn(buffer, "\n");
    if (n == 0) {
      parseGrid = 0;
      continue;
    }
    if (parseGrid) {
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
        if (nObject == MAX_ARRAY_SIZE) {
          printf("Too many objects\n");
          return EXIT_FAILURE;
        }
        ObjectBase *o = &objectPositions[nObject++];
        o->type = type;
        o->x = x;
        o->y = ny;
      }
      ny++;
    } else {
      for (int i = 0; i < n; ++i) {
        appendChar(moves, buffer[i]);
      }
    }
  }

  fclose(file);
  objectPositions = realloc(objectPositions, nObject * sizeof(ObjectBase));

  int part1 =
      solvePuzzle(objectPositions, nObject, nx, ny, startX, startY, moves, 1);
  printf("Part 1: %d\n", part1);
  int part2 =
      solvePuzzle(objectPositions, nObject, nx, ny, startX, startY, moves, 2);
  printf("Part 2: %d\n", part2);

  freeArrayList(moves);
  free(objectPositions);

  return 0;
}
