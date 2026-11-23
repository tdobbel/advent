#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_IMPLEMENTATION
#include "arena.h"
#define STRING_IMPLEMENTATION
#include "string8.h"
#define VECTOR_IMPLEMENTATION
#include "vector.h"
#define HASHMAP_IMPLEMENTATION
#include "hash_map.h"

typedef struct {
  u32 n;
  u32 *flow_rates;
  u8 *adj;
  u32 *min_dist;
} ValveNetwork;

#define MAX(a, b) ((a) > (b) ? (a) : (b))

u32 get_index(hash_map *name_map, string8 name);
void compute_distance_matrix(u32 *out, const u8 *adj, u32 n);
void solve_part1(u32 *max_pressure, const ValveNetwork *network, b8 *touched,
                 u32 src, u32 remaining_time, u32 total);
void solve_part2(u32 *max_pressure, const ValveNetwork *network, b8 *touched,
                 b8 elephant, u32 src[2], u32 remaining_time[2], u32 total);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  mem_arena *gpa = arena_create(KiB(500));

  string8 file = {0};
  str_read_file(gpa, &file, argv[1]);

  vector *line_vec = VEC_CREATE(string8);
  vector *word_vec = VEC_CREATE(string8);
  string8 splitted[2] = {0};

  hash_map *name_map = STRING_HASHMAP(u32);

  str_split(line_vec, str_trim(file), STR8_LIT("\n"));
  string8 *lines = VEC_CAST(line_vec, string8);

  u32 n_valve = line_vec->size;
  u32 *flow_rates = ALLOC_ARRAY(gpa, u32, n_valve);
  u8 *adjacency = ALLOC_ARRAY(gpa, u8, n_valve * n_valve);
  memset(adjacency, 0, n_valve * n_valve);

  for (u32 iv = 0; iv < line_vec->size; ++iv) {
    str_split_whitespace(word_vec, lines[iv]);
    string8 *words = VEC_CAST(word_vec, string8);
    u32 i = get_index(name_map, words[1]);
    str_split_once(splitted, words[4], STR8_LIT("="));
    string8 num_str = str_remove_suffix(splitted[1], STR8_LIT(";"));
    flow_rates[i] = str_parse_unsigned(num_str);
    for (u32 k = 9; k < word_vec->size; ++k) {
      string8 other = str_remove_suffix(words[k], STR8_LIT(","));
      u32 j = get_index(name_map, other);
      adjacency[i * n_valve + j] = 1;
    }
  }

  vector_deinit(line_vec);
  vector_deinit(word_vec);

  string8 start_valve = STR8_LIT("AA");
  u32 start_index = *(u32 *)hm_get_value(name_map, &start_valve);

  hm_deinit(name_map);

  u32 *min_dist = ALLOC_ARRAY(gpa, u32, n_valve * n_valve);
  compute_distance_matrix(min_dist, adjacency, n_valve);

  ValveNetwork network = (ValveNetwork){.n = n_valve,
                                        .flow_rates = flow_rates,
                                        .adj = adjacency,
                                        .min_dist = min_dist};
  b8 *touched = ALLOC_ARRAY(gpa, b8, n_valve);
  memset(touched, 0, n_valve);
  u32 part1 = 0;
  solve_part1(&part1, &network, touched, start_index, 30, 0);
  printf("Part 1: %u\n", part1);

  u32 src[2] = {start_index, start_index};
  u32 remaining[2] = {26, 26};
  u32 part2 = 0;
  solve_part2(&part2, &network, touched, 0, src, remaining, 0);
  printf("Part 2: %u\n", part2);

  arena_destroy(gpa);

  return EXIT_SUCCESS;
}

u32 get_index(hash_map *name_map, string8 name) {
  u32 length = name_map->size;
  kv_entry entry = hm_get_or_put(name_map, &name);
  if (entry.found_existing) {
    return *(u32 *)entry.value_ptr;
  }
  *(u32 *)entry.value_ptr = length;
  return length;
}

void compute_distance_matrix(u32 *out, const u8 *adj, u32 n) {
  for (u32 i = 0; i < n; i++) {
    for (u32 j = 0; j < n; j++) {
      if (i == j)
        out[i * n + j] = 0;
      else
        out[i * n + j] = adj[i * n + j] ? 1 : INT32_MAX;
    }
  }

  for (u32 k = 0; k < n; k++) {
    for (u32 i = 0; i < n; i++) {
      u32 ik = out[i * n + k];
      if (ik == INT32_MAX)
        continue;
      for (u32 j = 0; j < n; j++) {
        u32 d = ik + out[k * n + j];
        out[i * n + j] = MIN(out[i * n + j], d);
      }
    }
  }
}

void solve_part1(u32 *max_pressure, const ValveNetwork *network, b8 *touched,
                 u32 src, u32 remaining_time, u32 total) {
  if (remaining_time == 0) {
    *max_pressure = MAX(*max_pressure, total);
    return;
  }

  b8 moved = false;
  for (u32 dst = 0; dst < network->n; ++dst) {
    if (touched[dst] || network->flow_rates[dst] == 0)
      continue;
    u32 d = network->min_dist[src * network->n + dst];
    if (remaining_time <= d)
      continue;
    moved = true;
    u32 new_remaining = remaining_time - d - 1;
    u32 new_total = total + new_remaining * network->flow_rates[dst];
    touched[dst] = true;
    solve_part1(max_pressure, network, touched, dst, new_remaining, new_total);
    touched[dst] = false;
  }
  if (!moved) {
    solve_part1(max_pressure, network, touched, src, 0, total);
  }
}

void solve_part2(u32 *max_pressure, const ValveNetwork *network, b8 *touched,
                 b8 elephant, u32 src[2], u32 remaining_time[2], u32 total) {
  if (remaining_time[0] == 0 && remaining_time[1] == 0) {
    *max_pressure = MAX(*max_pressure, total);
    return;
  }

  b8 moved = false;
  for (u32 dst = 0; dst < network->n; ++dst) {
    if (touched[dst] || network->flow_rates[dst] == 0)
      continue;
    u32 d = network->min_dist[src[elephant] * network->n + dst];
    if (remaining_time[elephant] <= d)
      continue;
    moved = true;
    u32 new_remaining[2] = {remaining_time[0], remaining_time[1]};
    new_remaining[elephant] -= d + 1;
    u32 new_total = total + new_remaining[elephant] * network->flow_rates[dst];
    touched[dst] = true;
    u32 new_src[2] = {src[0], src[1]};
    new_src[elephant] = dst;
    solve_part2(max_pressure, network, touched, !elephant, new_src,
                new_remaining, new_total);
    touched[dst] = false;
  }
  if (!moved) {
    remaining_time[elephant] = 0;
    solve_part2(max_pressure, network, touched, !elephant, src, remaining_time,
                total);
  }
}
