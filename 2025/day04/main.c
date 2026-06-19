#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#include "arena.h"

typedef struct {
  u64 x, y;
} Position;

typedef struct {
  u64 ny, nx;
  string8 *data;
} Department;

b8 is_accessible(Department *d, u64 x, u64 y) {
  u64 x_start = x > 0 ? x - 1 : x;
  u64 x_stop = MIN(x + 1, d->nx - 1);
  u64 y_start = y > 0 ? y - 1 : y;
  u64 y_stop = MIN(y + 1, d->ny - 1);
  u8 count = 0;
  for (u64 ix = x_start; ix <= x_stop; ++ix) {
    for (u64 iy = y_start; iy <= y_stop; ++iy) {
      if (ix == x && iy == y)
        continue;
      if (d->data[iy].str[ix] == '@')
        count++;
    }
  }
  return count < 4;
}

u64 remove_accessible_rolls(Department *d, vector *pos_vec) {
  pos_vec->size = 0;
  for (u64 y = 0; y < d->ny; ++y) {
    for (u64 x = 0; x < d->nx; ++x) {
      if (d->data[y].str[x] != '@')
        continue;
      if (is_accessible(d, x, y)) {
        Position p = (Position){.x=x, .y=y};
        VEC_PUSH(pos_vec, Position, p);
      }
    }
  }

  u64 n_removed = pos_vec->size;
  Position *positions = pos_vec->data;
  for (u64 ip = 0; ip < n_removed; ++ip) {
    Position p = positions[ip];
    d->data[p.y].str[p.x] = '.';
  }
  return n_removed;
}


int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  mem_arena *perm_arena = arena_create(KiB(500));
  string8 file = {0};
  str_read_file(perm_arena, &file, argv[1]);

  vector *vec = VEC_ARENA_CREATE(perm_arena, string8);
  split(vec, file, STR8_LIT("\n"));
  u64 ny = vec->size;
  string8 *data = (string8 *)vec->data;
  u64 nx = data[0].size;

  Department d = (Department) { .ny=ny, .nx=nx, .data=data };
  vector *removed = VEC_ARENA_CREATE(perm_arena, Position);

  u64 total_removed = remove_accessible_rolls(&d, removed);
  printf("Part 1: %lu\n", total_removed);

  while (remove_accessible_rolls(&d, removed) > 0) {
    total_removed += removed->size;
  }
  printf("Part 2: %lu\n", total_removed);

  arena_destroy(perm_arena);

  return EXIT_SUCCESS;
}
