#include <stdio.h>
#include <string.h>

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#define BUFSIZE 256

typedef struct {
  u64 ny, nx;
  char **pixels;
} grid_t;

u64 count_trees(grid_t *grid, u64 n_right, u64 n_down) {
  u64 nTree = 0;
  u64 x = 0, y = 0;
  while (y < grid->ny) {
    if (grid->pixels[y][x] == '#')
      nTree++;
    x = (x + n_right) % grid->nx;
    y += n_down;
  }
  return nTree;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "Could not open file '%s'", argv[1]);
    return EXIT_FAILURE;
  }

  vector *vec = VEC_CREATE(char);
  char buffer[BUFSIZE];

  u64 nx, ny = 0;
  while (fgets(buffer, BUFSIZE, fp)) {
    nx = strcspn(buffer, "\n");
    for (u64 i = 0; i < nx; ++i) {
      VEC_PUSH(vec, char, buffer[i]);
    }
    ny++;
  }

  char **pixels = (char **)malloc(ny * sizeof(char *));
  pixels[0] = (char *)vec->data;
  for (u64 i = 1; i < ny; ++i) {
    pixels[i] = pixels[i - 1] + nx;
  }
  grid_t grid = {.nx = nx, .ny = ny, .pixels = pixels};

  u64 part1 = count_trees(&grid, 3, 1);
  printf("Part1: %lu\n", part1);

  u64 rights[5] = {1, 3, 5, 7, 1};
  u64 downs[5] = {1, 1, 1, 1, 2};
  u64 part2 = 1;
  for (u64 i = 0; i < 5; ++i) {
    part2 *= count_trees(&grid, rights[i], downs[i]);
  }
  printf("Part2: %lu\n", part2);

  fclose(fp);
  free(grid.pixels);
  vector_free(vec);

  return EXIT_SUCCESS;
}
