#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION
#define HASHMAP_IMPLEMENTATION

#include "hash_map.h"
#include "string8.h"
#include "vector.h"

typedef enum { Up, Right, Down, Left } Direction;

typedef struct {
  u32 ny, nx;
  u8 *heat_loss;
} CityMap;

typedef struct {
  u32 x, y;
  Direction heading;
  u8 counter;
} CrucibleState;

typedef struct {
  CrucibleState state;
  u32 heat_loss;
} Crucible;

u32 find_shortest_path(const CityMap *map, u8 min_block, u8 max_block);

Order compare_fn(const void *ctx, const void *a, const void *b) {
  CityMap *map = (CityMap *)ctx;
  Crucible *ca = (Crucible *)a;
  Crucible *cb = (Crucible *)b;
  u32 scoreA =
      ca->heat_loss + (map->nx - ca->state.x - 1) + (map->ny - ca->state.y - 1);
  u32 scoreB =
      cb->heat_loss + (map->nx - cb->state.x - 1) + (map->ny - cb->state.y - 1);
  if (scoreA < scoreB)
    return lt;
  if (scoreA > scoreB)
    return gt;
  return eq;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);
  vector *line_vec = VEC_CREATE(string8);
  str_split(line_vec, str_trim(file), STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->items;

  u32 ny = line_vec->size;
  u32 nx = lines[0].size;

  CityMap map = {.nx = nx, .ny = ny};
  map.heat_loss = (u8 *)malloc(nx * ny);

  u32 px = 0;
  for (u32 iy = 0; iy < ny; ++iy) {
    for (u32 ix = 0; ix < nx; ++ix) {
      map.heat_loss[px++] = lines[iy].str[ix] - '0';
    }
  }

  vector_deinit(line_vec);
  free(file.str);

  u32 part1 = find_shortest_path(&map, 1, 3);
  printf("Part 1: %u\n", part1);

  u32 part2 = find_shortest_path(&map, 4, 10);
  printf("Part 2: %u\n", part2);

  free(map.heat_loss);

  return EXIT_SUCCESS;
}

u32 find_shortest_path(const CityMap *map, u8 min_block, u8 max_block) {
  CrucibleState state0 =
      (CrucibleState){.x = 0, .y = 0, .heading = Right, .counter = 0};
  Crucible start = (Crucible){.state = state0, .heat_loss = 0};

  PriorityQueue *queue =
      PRIORITY_QUEUE_CREATE(Crucible, (void *)map, compare_fn);
  priority_queue_push(queue, &start);

  hash_map *visited = AUTO_HASHMAP(CrucibleState, b8);

  u32 sol = 0xffffffff;
  Direction next_dir[3] = {0};

  while (priority_queue_len(queue) > 0) {
    Crucible c = {0};
    priority_queue_pop(queue, &c);
    if (c.state.x == map->nx - 1 && c.state.y == map->ny - 1) {
      sol = MIN(sol, c.heat_loss);
      break;
    }
    next_dir[0] = (c.state.heading + 3) % 4;
    next_dir[1] = c.state.heading;
    next_dir[2] = (c.state.heading + 1) % 4;
    for (u8 k = 0; k < 3; ++k) {
      if (c.state.heading != next_dir[k] && c.state.counter > 0 &&
          c.state.counter < min_block)
        continue;
      u8 cntr = c.state.heading == next_dir[k] ? c.state.counter + 1 : 1;
      if (cntr > max_block)
        continue;
      CrucibleState next_state = (CrucibleState){.x = c.state.x,
                                                 .y = c.state.y,
                                                 .heading = next_dir[k],
                                                 .counter = cntr};
      switch (next_state.heading) {
      case Up:
        if (next_state.y == 0)
          continue;
        next_state.y--;
        break;
      case Right:
        if (next_state.x == map->nx - 1)
          continue;
        next_state.x++;
        break;
      case Down:
        if (next_state.y == map->ny - 1)
          continue;
        next_state.y++;
        break;
      case Left:
        if (next_state.x == 0)
          continue;
        next_state.x--;
        break;
      }
      kv_entry entry = hm_get_or_put(visited, &next_state);
      if (entry.found_existing)
        continue;
      *(b8 *)entry.value_ptr = 1;
      Crucible c_new = (Crucible){
          .state = next_state,
          .heat_loss =
              c.heat_loss +
              (u32)map->heat_loss[next_state.y * map->nx + next_state.x]};

      priority_queue_push(queue, &c_new);
    }
  }
  priority_queue_deinit(queue);
  hm_deinit(visited);

  return sol;
}
