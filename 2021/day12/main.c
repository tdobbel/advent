#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#define HASHMAP_IMPLEMENTATION
#include "hash_map.h"

#include "arena.h"

typedef struct {
  b8 small;
  u32 cntr;
  vector *conns;
} cave_node;

cave_node *get_cave(mem_arena *arena, hash_map *cave_map, string8 name) {
  kv_entry entry = hm_get_entry(cave_map, &name);
  if (entry.found_existing)
    return *(cave_node **)entry.value_ptr;
  cave_node *cave = ALLOC_STRUCT(arena, cave_node);
  cave->cntr = 0;
  cave->conns = VEC_ARENA_CREATE(arena, string8);
  cave->small = (name.str[0] >= 'a') && (name.str[0] <= 'z');
  hm_put(cave_map, &name, &cave);
  return cave;
}

void print_caves(hash_map *cave_map) {
  kv_iterator kvi = hm_iterator(cave_map);
  string8 neighbors = {0};
  while (get_next(&kvi)) {
    string8 name = *(string8 *)kvi.key_ptr;
    cave_node *cave = *(cave_node **)kvi.value_ptr;
    printf("Cave " STR8_FMT "\nConnected to: ", STR8_UNWRAP(name));
    str_join(NULL, &neighbors, STR8_LIT(" "), cave->conns->size,
             (string8 *)cave->conns->data);
    printf(STR8_FMT "\n", STR8_UNWRAP(neighbors));
    free(neighbors.str);
  }
}

void parse_caves(mem_arena *arena, string8 file, hash_map *cave_map) {
  vector *line_vec = VEC_CREATE(string8);
  split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;
  string8 edge[2];
  string8 delim = STR8_LIT("-");
  for (u32 i = 0; i < line_vec->size; ++i) {
    str_split_once(edge, lines[i], delim);
    cave_node *cave1 = get_cave(arena, cave_map, edge[0]);
    VEC_PUSH(cave1->conns, string8, edge[1]);
    cave_node *cave2 = get_cave(arena, cave_map, edge[1]);
    VEC_PUSH(cave2->conns, string8, edge[0]);
  }
  vector_free(line_vec);
}

u64 count_paths(hash_map *cave_map, string8 start, string8 end) {
  if (str_equal(start, end))
    return 1;
  cave_node *cave = HM_GET(cave_node *, cave_map, &start);
  if (cave->small && cave->cntr > 0)
    return 0;
  u64 total = 0;
  cave->cntr++;
  string8 *neighbors = (string8 *)cave->conns->data;
  for (u64 i = 0; i < cave->conns->size; ++i) {
    total += count_paths(cave_map, neighbors[i], end);
  }
  cave->cntr--;
  return total;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  string8 file = {0};
  mem_arena *perm_arena = arena_create(MiB(10));
  str_read_file(perm_arena, &file, argv[1]);

  hash_map *cave_map = STRING_HASHMAP_ARENA(perm_arena, cave_node *);
  parse_caves(perm_arena, file, cave_map);
  // print_caves(cave_map);

  string8 start = STR8_LIT("start");
  string8 end = STR8_LIT("end");
  u64 part1 = count_paths(cave_map, start, end);
  printf("Part 1: %lu\n", part1);

  arena_destroy(perm_arena);
  return EXIT_SUCCESS;
}
