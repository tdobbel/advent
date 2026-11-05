#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION
#define HASHMAP_IMPLEMENTATION

#include "hash_map.h"
#include "string8.h"
#include "vector.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
  string8 left, right;
} LRPair;

u64 solve_part1(hash_map *nodes, string8 moves, string8 start, b8 end_on_zzz);
void factorize(hash_map *factors, u64 num);
void update_lcm(hash_map *lcm_factors, hash_map *factors);
u64 upow(u64 base, u8 exp);

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
  string8 moves = lines[0];

  vector *start_vec = VEC_CREATE(string8);
  hash_map *nodes = STRING_HASHMAP(LRPair);

  for (u32 i = 2; i < line_vec->size && lines[i].size > 0; ++i) {
    string8 name = (string8){.str = lines[i].str, .size = 3};
    LRPair pair =
        (LRPair){.left = (string8){.str = lines[i].str + 7, .size = 3},
                 .right = (string8){.str = lines[i].str + 12, .size = 3}};
    hm_put(nodes, &name, &pair);
    if (name.str[2] == 'A')
      VEC_PUSH(start_vec, string8, name);
  }

  u64 part1 = solve_part1(nodes, moves, STR8_LIT("AAA"), 1);

  printf("Part 1: %lu\n", part1);

  hash_map *factors = AUTO_HASHMAP(u64, u8);
  hash_map *lcm_factors = AUTO_HASHMAP(u64, u8);
  string8 *start_nodes = (string8 *)start_vec->data;

  for (u32 i = 0; i < start_vec->size; ++i) {
    u64 num = solve_part1(nodes, moves, start_nodes[i], 0);
    factorize(factors, num);
    update_lcm(lcm_factors, factors);
  }

  u64 part2 = 1;
  kv_iterator kvi = hm_iterator(lcm_factors);
  while (get_next(&kvi)) {
    u64 base = *(u64 *)kvi.key_ptr;
    u8 exp = *(u8 *)kvi.value_ptr;
    part2 *= upow(base, exp);
  }

  printf("Part 2: %lu\n", part2);

  vector_free(line_vec);
  vector_free(start_vec);

  hm_deinit(nodes);
  hm_deinit(factors);
  hm_deinit(lcm_factors);

  free(file.str);

  return EXIT_SUCCESS;
}

u64 solve_part1(hash_map *nodes, string8 moves, string8 start, b8 end_on_zzz) {
  u64 n_moves = 0;
  u64 indx = 0;
  string8 zzz = STR8_LIT("ZZZ");
  string8 curr = start;
  while (end_on_zzz ? !str_equal(curr, zzz) : curr.str[2] != 'Z') {
    LRPair lr_pair = HM_GET(LRPair, nodes, &curr);
    u8 move = moves.str[indx];
    indx = (indx + 1) % moves.size;
    curr = move == 'R' ? lr_pair.right : lr_pair.left;
    n_moves++;
  }
  return n_moves;
}

void factorize(hash_map *factors, u64 num) {
  hm_reset(factors);
  u64 div = 2;
  while (num > 1) {
    u8 pow = 0;
    while (num % div == 0) {
      pow++;
      num /= div;
    }
    if (pow > 0) {
      hm_put(factors, &div, &pow);
    }
    div++;
  }
}

void update_lcm(hash_map *lcm_factors, hash_map *factors) {
  kv_iterator kvi = hm_iterator(factors);
  while (get_next(&kvi)) {
    u64 base = *(u64 *)kvi.key_ptr;
    u8 exp = *(u8 *)kvi.value_ptr;
    kv_entry entry = hm_get_or_put(lcm_factors, &base);
    if (!entry.found_existing) {
      *(u8 *)entry.value_ptr = exp;
    } else {
      *(u8 *)entry.value_ptr = MAX(*(u8 *)entry.value_ptr, exp);
    }
  }
}

u64 upow(u64 base, u8 exp) {
  u64 r = 1;
  for (u8 i = exp; i > 0; --i) {
    r *= base;
  }
  return r;
}
