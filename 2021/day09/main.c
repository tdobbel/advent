#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ARENA_IMPLEMENTATION
#include "arena.h"

#define BUFSIZE 1024

typedef u8 b8;
typedef enum { Up, Right, Down, Left } Direction;

typedef struct {
  u64 nx, ny;
  u8 *data;
} grid_info;

b8 get_neighor(grid_info grid, u64 x, u64 y, Direction d, u64 *xn, u64 *yn) {
  *xn = x;
  *yn = y;
  switch (d) {
  case Up:
    if (y == 0)
      return false;
    *yn = y - 1;
    return true;
  case Right:
    if (x >= grid.nx - 1)
      return false;
    *xn = x + 1;
    return true;
  case Down:
    if (y >= grid.ny - 1)
      return false;
    *yn = y + 1;
    return true;
  case Left:
    if (x == 0)
      return false;
    *xn = x - 1;
    return true;
  }
  return false;
}

int cmp(const void *a, const void *b) { return *(u64 *)b - *(u64 *)a; }

void count_basin(grid_info grid, u64 x, u64 y, b8 *visited, u64 *size) {
  (*size)++;
  visited[grid.nx * y + x] = true;
  u64 xn, yn;
  b8 level = grid.data[y * grid.nx + x];
  for (int k = 0; k < 4; ++k) {
    if (!get_neighor(grid, x, y, k, &xn, &yn))
      continue;
    u8 other = grid.data[yn * grid.nx + xn];
    if (!visited[yn * grid.nx + xn] && other > level && other < 9)
      count_basin(grid, xn, yn, visited, size);
  }
}

u64 solve(grid_info grid, b8 *visited, arena_vector *basins) {
  u64 total = 0;
  u64 xn, yn, size;
  for (u64 y = 0; y < grid.ny; ++y) {
    for (u64 x = 0; x < grid.nx; ++x) {
      u8 level = grid.data[y * grid.nx + x];
      b8 low_point = true;
      for (int k = 0; k < 4 && low_point; ++k) {
        if (!get_neighor(grid, x, y, k, &xn, &yn))
          continue;
        u8 other = grid.data[yn * grid.nx + xn];
        if (other <= level)
          low_point = false;
      }
      if (low_point) {
        total += level + 1;
        memset(visited, false, grid.nx * grid.ny);
        size = 0;
        count_basin(grid, x, y, visited, &size);
        AVEC_PUSH(basins, u64, size);
      }
    }
  }
  return total;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "Could not open file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  mem_arena *perm_arena = arena_create(KiB(100));
  arena_vector *vec = AVEC_CREATE(perm_arena, u8, 512);

  char buffer[BUFSIZE];
  u64 ny = 0;
  u64 nx = 0;

  while (fgets(buffer, BUFSIZE, fp)) {
    nx = strcspn(buffer, "\n");
    for (u64 i = 0; i < nx; ++i) {
      AVEC_PUSH(vec, u8, buffer[i] - '0');
    }
    ny++;
  }
  fclose(fp);

  grid_info grid = (grid_info){.nx = nx, .ny = ny, AVEC_FLATTEN(vec, u8)};
  b8 *visited = PUSH_ARRAY(perm_arena, b8, nx * ny);
  arena_vector *basins_vec = AVEC_CREATE(perm_arena, u64, 512);

  u64 part1 = solve(grid, visited, basins_vec);
  printf("Part 1: %lu\n", part1);

  u64 *basins = AVEC_FLATTEN(basins_vec, u64);
  qsort(basins, basins_vec->size, sizeof(u64), cmp);
  u64 part2 = 1;
  for (u64 i = 0; i < 3; ++i) {
    part2 *= basins[i];
  }
  printf("Part 2: %lu\n", part2);

  arena_destroy(perm_arena);

  return EXIT_SUCCESS;
}
