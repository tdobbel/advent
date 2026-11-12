#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION

#include "string8.h"
#include "vector.h"

typedef struct {
  u32 x, y;
} Position;

typedef struct {
  u32 ny, nx;
  u8 *grid;
  u32 *shortest;
} ElevationMap;

void elevation_map_reset(ElevationMap *self, Position start);
void elevation_map_find_shortest(ElevationMap *self, Position pos);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);
  vector *line_vec = VEC_CREATE(string8);
  str_split(line_vec, str_trim(file), STR8_LIT("\n"));
  string8 *lines = VEC_CAST(line_vec, string8);

  u32 ny = line_vec->size;
  u32 nx = lines[0].size;
  u32 px = 0;

  u8 *grid = (u8 *)malloc(nx * ny);
  u32 *shortest = (u32 *)malloc(sizeof(u32) * nx * ny);

  vector *as = VEC_CREATE(Position);
  Position start_pos = {0}, end_pos = {0};

  for (u32 y = 0; y < ny; ++y) {
    for (u32 x = 0; x < nx; ++x) {
      u8 c = lines[y].str[x];
      if (c == 'a') {
        Position pos = (Position){.x = x, .y = y};
        vector_push(as, &pos);
      } else if (c == 'S') {
        start_pos.x = x;
        start_pos.y = y;
        c = 'a';
      } else if (c == 'E') {
        end_pos.x = x;
        end_pos.y = y;
        c = 'z';
      }
      grid[px++] = c;
    }
  }

  vector_deinit(line_vec);

  ElevationMap em =
      (ElevationMap){.nx = nx, .ny = ny, .grid = grid, .shortest = shortest};
  elevation_map_reset(&em, start_pos);
  elevation_map_find_shortest(&em, start_pos);

  u32 sol = em.shortest[end_pos.y * em.nx + end_pos.x];
  printf("Part 1: %u\n", sol);

  for (u32 i = 0; i < as->size; ++i) {
    vector_get(as, &start_pos, i);
    elevation_map_reset(&em, start_pos);
    elevation_map_find_shortest(&em, start_pos);
    sol = MIN(sol, em.shortest[end_pos.y * em.nx + end_pos.x]);
  }
  printf("Part 2: %u\n", sol);

  vector_deinit(as);
  free(em.grid);
  free(em.shortest);
  free(file.str);

  return EXIT_SUCCESS;
}

void elevation_map_reset(ElevationMap *self, Position start) {
  memset(self->shortest, 0xff, sizeof(u32) * self->nx * self->ny);
  self->shortest[start.y * self->nx + start.x] = 0;
}

void elevation_map_find_shortest(ElevationMap *self, Position pos) {
  u32 length = self->shortest[pos.y * self->nx + pos.x];
  u8 elev = self->grid[pos.y * self->nx + pos.x];
  u32 n_next = 0;
  u32 next[4] = {0};
  if (pos.y > 0)
    next[n_next++] = (pos.y - 1) * self->nx + (pos.x - 0);
  if (pos.x < self->nx - 1)
    next[n_next++] = (pos.y + 0) * self->nx + (pos.x + 1);
  if (pos.y < self->ny - 1)
    next[n_next++] = (pos.y + 1) * self->nx + (pos.x + 0);
  if (pos.x > 0)
    next[n_next++] = (pos.y + 0) * self->nx + (pos.x - 1);
  for (u32 k = 0; k < n_next; ++k) {
    u32 px = next[k];
    if (self->grid[px] > elev + 1)
      continue;
    if (self->shortest[px] <= length + 1)
      continue;
    self->shortest[px] = length + 1;
    Position next_pos = (Position){.x = px % self->nx, .y = px / self->nx};
    elevation_map_find_shortest(self, next_pos);
  }
}
