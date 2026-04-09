#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHMAP_IMPLEMENTATION
#include "hash_map.h"

#define BUFSIZE 1024

typedef int32_t i32;
typedef int64_t i64;

typedef struct {
  i32 x, y, z;
} vec3;

typedef struct {
  b8 active;
  u8 active_neighbors;
} cell_status;

void update_cells(hash_map *active_cells) {
  hash_map *counter = AUTO_HASHMAP(vec3, cell_status);
  kv_iterator kvi = hm_iterator(active_cells);
  while (get_next(&kvi)) {
    vec3 pos = *(vec3 *)kvi.key_ptr;
    cell_status status = (cell_status){true, 0};
    for (i32 dx = -1; dx <= 1; ++dx) {
      for (i32 dy = -1; dy <= 1; ++dy) {
        for (i32 dz = -1; dz <= 1; ++dz) {
          if (dx == 0 && dy == 0 && dz == 0)
            continue;
          vec3 point = {pos.x + dx, pos.y + dy, pos.z + dz};
          kv_entry active = hm_get_entry(active_cells, &point);
          if (active.found_existing) {
            // do not update counter to avoid counting twice
            status.active_neighbors++;
            continue;
          }
          kv_entry other = hm_get_or_put(counter, &point);
          cell_status *other_status = (cell_status *)other.value_ptr;
          if (!other.found_existing) {
            other_status->active = false;
            other_status->active_neighbors = 0;
          }
          other_status->active_neighbors++;
        }
      }
    }
    hm_put(counter, &pos, &status);
  }
  hm_reset(active_cells);
  kvi = hm_iterator(counter);
  u8 dummy = 0;
  while (get_next(&kvi)) {
    cell_status *status = (cell_status *)kvi.value_ptr;
    b8 activate = !status->active && status->active_neighbors == 3;
    b8 remains_active = status->active && status->active_neighbors >= 2 &&
                        status->active_neighbors <= 3;
    if (activate || remains_active) {
      hm_put(active_cells, kvi.key_ptr, &dummy);
    }
  }
  hm_deinit(counter);
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
  hash_map *active_cells = AUTO_HASHMAP(vec3, u8);
  char buffer[BUFSIZE];
  i32 x, y = 0, z = 0;
  u8 dummy = 0;
  while (fgets(buffer, BUFSIZE, fp)) {
    i32 n = strcspn(buffer, "\n");
    for (x = 0; x < n; ++x) {
      if (buffer[x] == '.')
        continue;
      vec3 pos = {x, y, z};
      hm_put(active_cells, &pos, &dummy);
    }
    y++;
  }
  fclose(fp);

  // printf("%lu active cells\n", active_cells->size);
  for (u32 i = 0; i < 6; ++i) {
    update_cells(active_cells);
    // printf("%lu active cells\n", active_cells->size);
  }
  printf("Part1: %lu\n", active_cells->size);

  hm_deinit(active_cells);

  return EXIT_SUCCESS;
}
