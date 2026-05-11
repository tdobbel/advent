#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#define HASHMAP_IMPLEMENTATION
#include "hash_map.h"

void add_neighbor(hash_map *topo, u32 a, u32 b) {
  kv_entry entry = hm_get_or_put(topo, &a);
  vector **vptr = entry.value_ptr;
  if (!entry.found_existing) {
    *vptr = VEC_CREATE(u32);
  }
  VEC_PUSH(*vptr, u32, b);
}

u32 get_name_id(hash_map *name_map, string8 name, u32 *id) {
  kv_entry entry = hm_get_or_put(name_map, &name);
  if (!entry.found_existing) {
    *(u32 *)entry.value_ptr = *id;
    (*id)++;
  }
  return *(u32 *)entry.value_ptr;
}

u64 count_path(hash_map *topo, hash_map *cache, u32 src, u32 dst) {
  kv_entry entry;
  entry = hm_get_entry(cache, &src);
  if (entry.found_existing)
    return *(u64 *)entry.value_ptr;
  if (src == dst)
    return 1;
  u64 count = 0;
  entry = hm_get_entry(topo, &src);
  if (!entry.found_existing)
    return 0;
  vector **vptr = (vector **)entry.value_ptr;
  u32 *nodes = (u32 *)(*vptr)->data;
  for (u64 i = 0; i < (*vptr)->size; ++i) {
    count += count_path(topo, cache, nodes[i], dst);
  }
  hm_put(cache, &src, &count);
  return count;
}

u64 get_path(hash_map *topo, u64 n_node, const u32 *nodes) {
  u64 total = 1;
  hash_map *cache = AUTO_HASHMAP(u32, u64);
  for (u64 i = 0; i < n_node - 1; ++i) {
    total *= count_path(topo, cache, nodes[i], nodes[i + 1]);
    if (total == 0)
      break;
    hm_reset(cache);
  }
  hm_deinit(cache);
  return total;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  string8 fc = {0};
  str_read_file(argv[1], &fc);

  hash_map *name_map = STRING_HASHMAP(u32);
  hash_map *topo = AUTO_HASHMAP(u32, vector *);

  vector *line_vec = split(fc, STR8_LIT("\n"));
  string8 *lines = line_vec->data;
  string8 splitted[2];
  u32 iname = 0;
  for (u64 i = 0; i < line_vec->size; ++i) {
    str_split_once(splitted, lines[i], STR8_LIT(": "));
    u32 ia = get_name_id(name_map, splitted[0], &iname);
    vector *neighbors_vec = split_whitespace(splitted[1]);
    string8 *neighbors = (string8 *)neighbors_vec->data;
    for (u32 j = 0; j < neighbors_vec->size; ++j) {
      u32 ib = get_name_id(name_map, neighbors[j], &iname);
      add_neighbor(topo, ia, ib);
    }
    vector_free(neighbors_vec);
  }
  vector_free(line_vec);

  string8 key;
  key = STR8_LIT("you");
  u32 you = HM_GET(u32, name_map, &key);
  key = STR8_LIT("out");
  u32 out = HM_GET(u32, name_map, &key);
  key = STR8_LIT("svr");
  u32 svr = HM_GET(u32, name_map, &key);
  key = STR8_LIT("dac");
  u32 dac = HM_GET(u32, name_map, &key);
  key = STR8_LIT("fft");
  u32 fft = HM_GET(u32, name_map, &key);

  u32 route0[2] = {you, out};
  u64 part1 = get_path(topo, 2, route0);
  printf("Part 1: %lu\n", part1);

  u32 route1[4] = {svr, dac, fft, out};
  u64 path1 = get_path(topo, 4, route1);
  u32 route2[4] = {svr, fft, dac, out};
  u64 path2 = get_path(topo, 4, route2);

  printf("Part 2: %lu\n", path1 + path2);

  kv_iterator kvi = hm_iterator(topo);
  while (get_next(&kvi)) {
    vector **vptr = kvi.value_ptr;
    vector_free(*vptr);
  }

  hm_deinit(name_map);
  hm_deinit(topo);
  free(fc.str);
}
