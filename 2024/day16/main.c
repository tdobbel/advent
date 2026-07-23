#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#include "arena.h"

typedef enum Direction direction;

enum Direction { East, South, West, North };

typedef struct {
  direction heading;
  u64 cost;
  u32 length;
  u32 *cells;
} path_t;

typedef struct {
  u32 ny, nx;
  u8 **grid;
} maze_t;

void find_all_shortest(maze_t maze, vector *queue, vector *found,
                       u64 *min_cost);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  mem_arena *perm_arena = arena_create(MiB(10));

  string8 file = {0};
  str_read_file(perm_arena, &file, argv[1]);
  vector *line_vec = VEC_ARENA_CREATE(perm_arena, string8);
  split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;
  if (lines[line_vec->size - 1].size == 0)
    line_vec->size--;
  u64 nx = lines[0].size;
  u64 ny = line_vec->size;

  u32 startX = 0, startY = 0;

  u8 **grid = ALLOC_ARRAY(perm_arena, u8 *, ny);
  for (u64 y = 0; y < ny; ++y) {
    grid[y] = lines[y].str;
    for (u64 x = 0; x < nx; ++x) {
      if (grid[y][x] == 'S') {
        startX = x;
        startY = y;
      }
    }
  }

  maze_t maze = (maze_t){.ny = ny, .nx = nx, .grid = grid};
  vector *queue = VEC_ARENA_CREATE(perm_arena, path_t);
  u32 *cells = (u32 *)malloc(sizeof(u32));
  cells[0] = startY * maze.ny + startX;
  path_t path =
      (path_t){.heading = East, .cost = 0, .length = 1, .cells = cells};
  VEC_PUSH(queue, path_t, path);

  vector *found = VEC_ARENA_CREATE(perm_arena, path_t);
  u64 min_cost = UINTMAX_MAX;

  find_all_shortest(maze, queue, found, &min_cost);
  printf("Part 1: %lu\n", min_cost);

  path_t *paths = (path_t *)found->data;
  b8 *visited = ALLOC_ARRAY(perm_arena, b8, nx * ny);
  memset(visited, 0, nx * ny);
  for (u64 i = 0; i < found->size; ++i) {
    if (paths[i].cost == min_cost) {
      for (u32 ic = 0; ic < paths[i].length; ++ic) {
        u32 c = paths[i].cells[ic];
        visited[c] = 1;
      }
    }
    free(paths[i].cells);
  }
  u32 part2 = 0;
  for (u32 i = 0; i < nx * ny; ++i) {
    part2 += visited[i];
  }
  printf("Part 2: %u\n", part2);

  arena_destroy(perm_arena);

  return EXIT_SUCCESS;
}

void find_all_shortest(maze_t maze, vector *queue, vector *found,
                       u64 *min_cost) {
  u64 tol = 1000;
  u64 *best_score = malloc(sizeof(u64) * maze.ny * maze.nx);
  for (u32 i = 0; i < maze.nx * maze.ny; ++i) {
    best_score[i] = UINTMAX_MAX - tol;
  }
  while (queue->size > 0) {
    path_t *paths = (path_t *)queue->data;
    u64 imin = 0;
    u64 current_min = paths[0].cost;
    for (u64 i = 0; i < queue->size; ++i) {
      if (paths[i].cost < current_min) {
        imin = i;
        current_min = paths[i].cost;
      }
    }
    if (current_min > *min_cost)
      break;
    path_t p = paths[imin];
    vector_remove(queue, imin);
    u32 x = p.cells[p.length - 1];
    best_score[x] = MIN(best_score[x], p.cost);
    u32 y = x / maze.nx;
    x -= maze.nx * y;
    if (maze.grid[y][x] == 'E') {
      *min_cost = MIN(*min_cost, p.cost);
      VEC_PUSH(found, path_t, p);
      continue;
    }
    direction dirs[3] = {(p.heading + 3) % 4, p.heading, (p.heading + 1) % 4};
    for (u32 i = 0; i < 3; ++i) {
      u32 x_next = x;
      u32 y_next = y;
      switch (dirs[i]) {
      case East:
        x_next++;
        break;
      case South:
        y_next++;
        break;
      case West:
        x_next--;
        break;
      case North:
        y_next--;
        break;
      }
      if (maze.grid[y_next][x_next] == '#')
        continue;
      u32 next_cell = y_next * maze.nx + x_next;
      u64 cost = p.cost + 1;
      if (p.heading != dirs[i])
        cost += 1000;
      if (cost > best_score[next_cell] + tol)
        continue;
      u32 *cells = malloc(sizeof(u32) * (p.length + 1));
      memcpy(cells, p.cells, p.length * sizeof(u32));
      cells[p.length] = next_cell;
      path_t new_path = (path_t){.heading = dirs[i],
                                 .length = p.length + 1,
                                 .cost = cost,
                                 .cells = cells};
      VEC_PUSH(queue, path_t, new_path);
    }
    free(p.cells);
  }
  free(best_score);
  path_t *paths = (path_t *)queue->data;
  for (u64 i = 0; i < queue->size; ++i) {
    free(paths[i].cells);
  }
}
