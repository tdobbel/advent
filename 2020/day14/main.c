#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHMAP_IMPLEMENTATION
#include "hashmap.h"

#define MASK_SIZE 36
#define BUFSIZE 64

typedef struct {
  u64 ones, zeros;
} bitmask;

void parse_mask(bitmask *mask, char *line) {
  mask->ones = 0;
  mask->zeros = 0;
  for (u64 i = 0; i < MASK_SIZE; ++i) {
    if (line[i] == 'X')
      continue;
    u64 bit = (1L << (MASK_SIZE - i - 1));
    if (line[i] == '1') {
      mask->ones |= bit;
    } else {
      mask->zeros |= bit;
    }
  }
  assert(mask->ones < (1L << 36));
  assert(mask->ones < (1L << 36));
}

void set_memory(hash_map *hm, const char *mask, u64 addr, u64 value,
                u32 index) {
  if (index == MASK_SIZE) {
    hm_put(hm, &addr, &value);
    return;
  }
  if (mask[index] != 'X') {
    set_memory(hm, mask, addr, value, index + 1);
    return;
  }
  u64 bit = 1L << (MASK_SIZE - index - 1);
  set_memory(hm, mask, addr | bit, value, index + 1);
  set_memory(hm, mask, addr & (~bit), value, index + 1);
}

u64 hm_sum(hash_map *hm) {
  kv_iterator kvi = hm_iterator(hm);
  u64 sum = 0;
  while (get_next(&kvi)) {
    sum += *(u64 *)kvi.value_ptr;
  }
  return sum;
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

  bitmask mask = (bitmask){.ones = 0, .zeros = 0};
  hash_map *mem = HM_CREATE(u64, u64);
  hash_map *hm = HM_CREATE(u64, u64);

  char buffer[BUFSIZE];
  char addr_mask[MASK_SIZE];

  while (fgets(buffer, BUFSIZE, fp)) {
    u32 n = strcspn(buffer, "\n");
    u32 ieq = strcspn(buffer, "=");
    buffer[n] = '\0';
    if (eql("mask", buffer, 4)) {
      parse_mask(&mask, buffer + ieq + 2);
      memcpy(addr_mask, buffer + ieq + 2, MASK_SIZE);
      continue;
    }
    u64 index = 0;
    for (u32 i = 4; i < ieq - 2; ++i) {
      index = index * 10 + (u64)(buffer[i] - '0');
    }
    u64 address = index; // address used for part 2
    u64 value = 0;
    for (u32 i = ieq + 2; i < n; ++i) {
      value = value * 10 + (u64)(buffer[i] - '0');
    }
    for (u32 i = 0; i < MASK_SIZE; ++i) {
      if (addr_mask[i] == '1') {
        address |= 1L << (MASK_SIZE - i - 1);
      }
    }
    // Solve part 2 before modifying value
    set_memory(hm, addr_mask, address, value, 0);

    // Part 1
    value |= mask.ones;
    value &= ~mask.zeros;
    hm_put(mem, &index, &value);
  }
  fclose(fp);

  printf("Part1: %lu\n", hm_sum(mem));
  printf("Part2: %lu\n", hm_sum(hm));

  hm_deinit(mem);
  hm_deinit(hm);

  return EXIT_SUCCESS;
}
