#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#include "arena.h"

typedef struct {
  u32 pos[2];
  u32 score;
} path_head;

typedef struct {
  u64 ny, nx;
  u8 **cavern;
  u32 *shortest;
} maze_grid;

u32 find_shortest_path(maze_grid *puzzle) {
  vector *queue = VEC_CREATE(path_head);
  path_head start = (path_head){.pos = {0, 0}, .score = 0};
  VEC_PUSH(queue, path_head, start);
  u32 max_score = puzzle->nx * puzzle->ny * 9;
  memset(puzzle->shortest, max_score, puzzle->nx * puzzle->ny * sizeof(u32));
  puzzle->shortest[0] = 0;
  i32 dx[4] = {-1, 0, 1, 0};
  i32 dy[4] = {0, -1, 0, 1};
  u32 best = max_score;
  while (queue->size > 0) {
    u32 imin = 0;
    u32 min_score = max_score;
    path_head curr = {0};
    // Find path with smallest score
    for (u32 i = 0; i < queue->size; ++i) {
      path_head *head = (path_head *)vector_get(queue, i);
      u32 score =
          head->score + puzzle->nx - 1 - head->pos[0] + puzzle->ny - 1 - head->pos[1];
      if (score < min_score) {
        min_score = score;
        curr = *head;
        imin = i;
      }
    }
    vector_remove(queue, imin);
    if (curr.pos[0] == puzzle->nx - 1 && curr.pos[1] == puzzle->ny - 1) {
      best = MIN(best, min_score);
      continue;
    }
    // Loop through neighbors, if path shorter than previous one, add to queue
    for (u32 k = 0; k < 4; ++k) {
      i32 x = (i32)curr.pos[0] + dx[k];
      i32 y = (i32)curr.pos[1] + dy[k];
      if (x < 0 || x >= puzzle->nx || y < 0 || y >= puzzle->ny)
        continue;
      u32 new_score = curr.score + (u32)(puzzle->cavern[y][x] - '0');
      if (new_score >= puzzle->shortest[puzzle->nx * y + x])
        continue;
      puzzle->shortest[puzzle->nx * y + x] = new_score;
      path_head new_path = (path_head){.pos = {x, y}, .score = new_score};
      VEC_PUSH(queue, path_head, new_path);
    }
  }
  vector_free(queue);
  return best;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  mem_arena *perm_arena = arena_create(MiB(1));

  string8 file = {0};
  str_read_file(perm_arena, &file, argv[1]);
  vector *line_vec = VEC_ARENA_CREATE(perm_arena, string8);
  split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;
  u64 ny = line_vec->size;
  u64 nx = lines[0].size;

  u8 **cavern = ALLOC_ARRAY(perm_arena, u8 *, ny);
  for (u64 i = 0; i < ny; ++i) {
    cavern[i] = lines[i].str;
  }
  u32 *shortest = ALLOC_ARRAY(perm_arena, u32, nx * ny);
  maze_grid puzzle =
      (maze_grid){.ny = ny, .nx = nx, .cavern = cavern, .shortest = shortest};
  u32 best = find_shortest_path(&puzzle);
  printf("Part 1: %u\n", best);

  // for (u32 y = 0; y < ny; ++y) {
  //   for (u32 x = 0; x < nx; ++x) {
  //     printf(" %02u ", shortest[nx * y + x]);
  //   }
  //   printf("\n");
  // }

  arena_destroy(perm_arena);

  return EXIT_SUCCESS;
}
