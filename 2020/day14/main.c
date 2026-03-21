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
  char buffer[BUFSIZE];
  while (fgets(buffer, BUFSIZE, fp)) {
    u32 n = strcspn(buffer, "\n");
    u32 ieq = strcspn(buffer, "=");
    buffer[n] = '\0';
    if (eql("mask", buffer, 4)) {
      parse_mask(&mask, buffer + ieq + 2);
      continue;
    }
    u64 index = 0;
    for (u32 i = 4; i < ieq - 2; ++i) {
      index = index * 10 + (u64)(buffer[i] - '0');
    }
    u64 value = 0;
    for (u32 i = ieq + 2; i < n; ++i) {
      value = value * 10 + (u64)(buffer[i] - '0');
    }
    value |= mask.ones;
    value &= ~mask.zeros;
    hm_put(mem, &index, &value);
  }
  fclose(fp);

  kv_iterator kvi = hm_iterator(mem);
  u64 part1 = 0;
  while (get_next(&kvi)) {
    part1 += *(u64 *)kvi.value_ptr;
  }
  printf("Part1: %lu\n", part1);

  hm_deinit(mem);

  return EXIT_SUCCESS;
}
