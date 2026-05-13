#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#define HASHMAP_IMPLEMENTATION
#include "hash_map.h"

typedef struct {
  string8 name;
  vector *parents;
  vector *children;
  vector *quantity;
} Bag;

Bag *get_bag(hash_map *hm, string8 name) {
  kv_entry entry = hm_get_or_put(hm, &name);
  Bag **vp = (Bag **)entry.value_ptr;
  if (entry.found_existing)
    return (*vp);
  *vp = malloc(sizeof(Bag));
  Bag *bag = *vp;
  bag->name = name;
  bag->parents = VEC_CREATE(Bag *);
  bag->children = VEC_CREATE(Bag *);
  bag->quantity = VEC_CREATE(u64);
  return bag;
}

void free_bags(hash_map *name_map) {
  kv_iterator kvi = hm_iterator(name_map);
  while (get_next(&kvi)) {
    Bag *bag = *(Bag **)kvi.value_ptr;
    vector_free(bag->parents);
    vector_free(bag->children);
    vector_free(bag->quantity);
    free(bag);
  }
}

void read_bag_info(string8 fc, hash_map *name_map) {
  vector *line_vec = VEC_CREATE(string8);
  split(line_vec, fc, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;

  string8 splitted[2] = { {0}, {0} };
  string8 final_s = STR8_LIT("s");
  string8 final_dot = STR8_LIT(".");
  vector *children_vec = VEC_CREATE(string8);

  for (u64 il = 0; il < line_vec->size; ++il) {
    str_split_once(splitted, lines[il], STR8_LIT(" contain "));
    Bag *parent = get_bag(name_map, str_remove_suffix(splitted[0], final_s));
    string8 rhs = str_remove_suffix(splitted[1], final_dot);
    if (str_equal(rhs, STR8_LIT("no other bags")))
      continue;
    split(children_vec, rhs, STR8_LIT(", "));
    string8 *children = (string8 *)children_vec->data;
    u64 qty;
    for (u64 ic = 0; ic < children_vec->size; ++ic) {
      string8 child_str = str_remove_suffix(children[ic], final_s);
      u64 indx;
      for (indx = 0; indx < child_str.size && child_str.str[indx] != ' ';
           indx++) {
        ;
      }
      str_parse_unsigned(&qty, (string8){.str = child_str.str, .size = indx});
      string8 child_name = (string8){.str = child_str.str + indx + 1,
                                     .size = child_str.size - indx - 1};
      Bag *child = get_bag(name_map, child_name);
      VEC_PUSH(child->parents, Bag *, parent);
      VEC_PUSH(parent->children, Bag *, child);
      VEC_PUSH(parent->quantity, u64, qty);
    }
  }

  vector_free(children_vec);
  vector_free(line_vec);
}

void solve_part1(hash_map *set, Bag *bag) {
  Bag **parents = (Bag **)bag->parents->data;
  u64 dummy = 0;
  for (u64 ip = 0; ip < bag->parents->size; ++ip) {
    hm_put(set, &parents[ip]->name, &dummy);
    solve_part1(set, parents[ip]);
  }
}

u64 solve_part2(Bag *bag, hash_map *cache) {
  kv_entry entry = hm_get_entry(cache, &bag->name);
  if (entry.found_existing)
    return *(u64 *)entry.value_ptr;
  Bag **children = (Bag **)bag->children->data;
  u64 *quantity = (u64 *)bag->quantity->data;
  u64 n_bags = 0;
  for (u64 i = 0; i < bag->children->size; ++i) {
    n_bags += quantity[i] * (1 + solve_part2(children[i], cache));
  }
  hm_put(cache, &bag->name, &n_bags);
  return n_bags;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file");
    return EXIT_FAILURE;
  }
  string8 fc = {0};
  str_read_file(&fc, argv[1]);

  hash_map *name_map = STRING_HASHMAP(Bag *);
  read_bag_info(fc, name_map);

  Bag *start_bag = HM_GET(Bag *, name_map, &STR8_LIT("shiny gold bag"));
  hash_map *set = STRING_HASHMAP(u64);
  solve_part1(set, start_bag);

  printf("Part 1: %lu\n", set->size);

  hm_reset(set);

  u64 part2 = solve_part2(start_bag, set);
  printf("Part 2: %lu\n", part2);

  hm_deinit(set);

  free_bags(name_map);
  hm_deinit(name_map);

  free(fc.str);
  return EXIT_SUCCESS;
}
