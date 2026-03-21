#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHMAP_IMPLEMENTATION
#include "collections.h"

#define BUFSIZE 1024

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input\n");
    return EXIT_FAILURE;
  }
  u64 turn = 1;
  // hash_map *hm = HM_CREATE(u64, u64);
  hash_map *hm = hm_init(1024 * 1042, sizeof(u64), sizeof(u64));
  char *start_nums = argv[1];
  u64 num = 0;
  for (u32 i = 0; i < strlen(start_nums); ++i) {
    if (start_nums[i] == ',') {
      hm_put(hm, &num, &turn);
      num = 0;
      turn++;
    } else {
      num = num * 10 + (u64)(start_nums[i] - '0');
    }
  }
  while (turn < 2020) {
    kv_entry entry = hm_get_or_put(hm, &num);
    if (!entry.found_existing) {
      num = 0;
    } else {
      u64 last_turn = *(u64 *)entry.value_ptr;
      num = turn - last_turn;
    }
    *(u64 *)entry.value_ptr = turn;
    turn++;
  }

  printf("Part 1: %lu\n", num);

  while (turn < 30000000) {
    kv_entry entry = hm_get_or_put(hm, &num);
    if (!entry.found_existing) {
      num = 0;
    } else {
      u64 last_turn = *(u64 *)entry.value_ptr;
      num = turn - last_turn;
    }
    *(u64 *)entry.value_ptr = turn;
    turn++;
  }

  printf("Part 2: %lu\n", num);

  hm_deinit(hm);

  return EXIT_SUCCESS;
}
