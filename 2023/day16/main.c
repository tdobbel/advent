#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION
#define HASHMAP_IMPLEMENTATION

#include "hash_map.h"
#include "string8.h"
#include "vector.h"

typedef struct {
  u64 ny, nx;
  u8 **grid;
} Contraption;

typedef enum { Up, Right, Down, Left } Direction;

typedef struct {
  u32 x, y;
  Direction heading;
} BeamState;

#define MAX(a, b) ((a) > (b) ? (a) : (b))

void contraption_init(Contraption *contraption, string8 *lines, u64 ny);
void propagate_beam(Contraption *contraption, BeamState bs, hash_map *visited);
b8 beam_next(BeamState *next_state, Contraption *contraption, BeamState bs,
             Direction d);
u32 count_visited(Contraption *contraption, hash_map *visited, b8 **touched);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);

  vector *line_vec = VEC_CREATE(string8);
  str_split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;

  Contraption contraption = {0};
  contraption_init(&contraption, lines, line_vec->size);

  hash_map *visited = AUTO_HASHMAP(BeamState, u8);
  BeamState start = (BeamState){.x = 0, .y = 0, .heading = Right};
  propagate_beam(&contraption, start, visited);

  b8 *touched = NULL;
  u32 part1 = count_visited(&contraption, visited, &touched);

  printf("Part 1: %u\n", part1);

  // Part 2
  u32 part2 = 0;
  u32 total;
  for (u32 y = 0; y < contraption.ny; ++y) {
    // Left
    hm_reset(visited);
    start.x = 0;
    start.y = y;
    start.heading = Right;
    propagate_beam(&contraption, start, visited);
    total = count_visited(&contraption, visited, &touched);
    part2 = MAX(part2, total);

    // Right
    hm_reset(visited);
    start.x = contraption.nx - 1;
    start.y = y;
    start.heading = Left;
    propagate_beam(&contraption, start, visited);
    total = count_visited(&contraption, visited, &touched);
    part2 = MAX(part2, total);
  }
  for (u32 x = 0; x < contraption.nx; ++x) {
    // Top
    hm_reset(visited);
    start.x = x;
    start.y = 0;
    start.heading = Down;
    propagate_beam(&contraption, start, visited);
    total = count_visited(&contraption, visited, &touched);
    part2 = MAX(part2, total);

    // Down
    hm_reset(visited);
    start.x = x;
    start.y = contraption.ny - 1;
    start.heading = Up;
    propagate_beam(&contraption, start, visited);
    total = count_visited(&contraption, visited, &touched);
    part2 = MAX(part2, total);
  }

  printf("Part 2: %u\n", part2);

  free(touched);
  hm_deinit(visited);
  free(contraption.grid);
  vector_free(line_vec);
  free(file.str);

  return EXIT_SUCCESS;
}

void contraption_init(Contraption *contraption, string8 *lines, u64 ny) {
  contraption->ny = ny;
  contraption->nx = lines[0].size;
  contraption->grid = (u8 **)malloc(sizeof(u8 *) * ny);
  for (u32 i = 0; i < ny; ++i) {
    contraption->grid[i] = lines[i].str;
  }
}

b8 beam_next(BeamState *next_state, Contraption *contraption, BeamState bs,
             Direction d) {
  next_state->heading = d;
  next_state->x = bs.x;
  next_state->y = bs.y;
  switch (d) {
  case Up:
    if (bs.y == 0)
      return 0;
    next_state->y--;
    break;
  case Right:
    if (bs.x == contraption->nx - 1)
      return 0;
    next_state->x++;
    break;
  case Down:
    if (bs.y == contraption->ny - 1)
      return 0;
    next_state->y++;
    break;
  case Left:
    if (bs.x == 0)
      return 0;
    next_state->x--;
    break;
  }
  return 1;
}

void propagate_beam(Contraption *contraption, BeamState bs, hash_map *visited) {
  u8 dummy = 1;
  hm_put(visited, &bs, &dummy);
  Direction next_dir[2];
  u32 n_dir = 0;

  switch (contraption->grid[bs.y][bs.x]) {
  case '.':
    next_dir[n_dir++] = bs.heading;
    break;
  case '-':
    switch (bs.heading) {
    case Left:
    case Right:
      next_dir[n_dir++] = bs.heading;
      break;
    case Up:
    case Down:
      next_dir[n_dir++] = (bs.heading + 1) % 4;
      next_dir[n_dir++] = (bs.heading + 3) % 4;
      break;
    }
    break;
  case '|':
    switch (bs.heading) {
    case Up:
    case Down:
      next_dir[n_dir++] = bs.heading;
      break;
    case Left:
    case Right:
      next_dir[n_dir++] = (bs.heading + 1) % 4;
      next_dir[n_dir++] = (bs.heading + 3) % 4;
      break;
    }
    break;
  case '\\':
    switch (bs.heading) {
    case Up:
    case Down:
      next_dir[n_dir++] = (bs.heading + 3) % 4;
      break;
    case Left:
    case Right:
      next_dir[n_dir++] = (bs.heading + 1) % 4;
      break;
    }
    break;
  case '/':
    switch (bs.heading) {
    case Up:
    case Down:
      next_dir[n_dir++] = (bs.heading + 1) % 4;
      break;
    case Left:
    case Right:
      next_dir[n_dir++] = (bs.heading + 3) % 4;
      break;
    }
    break;
  default:
    fprintf(stderr, "This should not be happening\n");
    exit(1);
  }
  BeamState new_state = {0};
  for (u32 i = 0; i < n_dir; ++i) {
    if (!beam_next(&new_state, contraption, bs, next_dir[i]))
      continue;
    if (hm_contains_key(visited, &new_state))
      continue;
    propagate_beam(contraption, new_state, visited);
  }
}

u32 count_visited(Contraption *contraption, hash_map *visited, b8 **touched) {
  u32 size = contraption->ny * contraption->nx;
  if (*touched == NULL) {
    *touched = (b8 *)malloc(size);
  }
  memset(*touched, 0, size);
  kv_iterator kvi = hm_iterator(visited);
  while (get_next(&kvi)) {
    BeamState *bs = (BeamState *)kvi.key_ptr;
    (*touched)[bs->y * contraption->nx + bs->x] = 1;
  }
  u32 total = 0;
  for (u32 i = 0; i < size; ++i) {
    if ((*touched)[i])
      total++;
  }
  return total;
}
