#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STRING_IMPLEMENTATION
#include "string8.h"
#define VECTOR_IMPLEMENTATION
#include "vector.h"

u8 to_mask(string8 signal) {
  u8 bitmask = 0;
  for (u64 i = 0; i < signal.size; ++i) {
    u8 p = signal.str[i] - 'a';
    bitmask |= (1 << p);
  }
  return bitmask;
}

void to_signal(char signal[8], u8 bitmask) {
  int cntr = 0;
  for (int i = 0; i < 7; ++i) {
    if (bitmask & (1 << i)) {
      signal[cntr++] = 'a' + i;
    }
    signal[cntr] = '\0';
  }
}

void find_mapping(u8 mapping[10], string8 *signals) {
  memset(mapping, 0, 10);
  u8 bitmasks[10];
  u64 i;
  // Find numbers with unique number of segments
  for (i = 0; i < 10; ++i) {
    bitmasks[i] = to_mask(signals[i]);
    switch (signals[i].size) {
    case 2:
      mapping[1] = bitmasks[i];
      break;
    case 3:
      mapping[7] = bitmasks[i];
      break;
    case 4:
      mapping[4] = bitmasks[i];
      break;
    case 7:
      mapping[8] = bitmasks[i];
      break;
    default:
      continue;
    }
  }
  // Identify 6-segment numbers
  for (i = 0; i < 10; ++i) {
    if (signals[i].size != 6)
      continue;
    if ((bitmasks[i] & mapping[4]) == mapping[4]) {
      mapping[9] = bitmasks[i];
    } else if ((bitmasks[i] | mapping[1]) == mapping[8]) {
      mapping[6] = bitmasks[i];
    } else {
      mapping[0] = bitmasks[i];
    }
  }
  // Identify 5-segment numbers
  for (i = 0; i < 10; ++i) {
    if (signals[i].size != 5)
      continue;
    if ((bitmasks[i] & mapping[6]) == bitmasks[i]) {
      mapping[5] = bitmasks[i];
    } else if ((bitmasks[i] | mapping[1]) == bitmasks[i]) {
      mapping[3] = bitmasks[i];
    } else {
      mapping[2] = bitmasks[i];
    }
  }
  // Sanity check
  // char check[8];
  // for (i = 0; i < 10; ++i) {
  //   to_signal(check, mapping[i]);
  //   printf("%lu => %hu (%s)\n", i, mapping[i], check);
  // }
}

u8 bitmask_to_number(u8 mapping[10], u8 bitmask) {
  for (u8 i = 0; i < 10; ++i) {
    if (mapping[i] == bitmask)
      return i;
  }
  return 10;
}

u64 decode_rhs(u8 mapping[10], string8 *rhs) {
  u64 num = 0;
  for (u64 i = 0; i < 4; ++i) {
    num = num * 10 + (u64)bitmask_to_number(mapping, to_mask(rhs[i]));
  }
  return num;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  string8 fc = {0};
  str_read_file(&fc, argv[1]);

  vector *line_vec = VEC_CREATE(string8);
  split(line_vec, fc, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;
  string8 splitted[2];
  vector *rhs = VEC_CREATE(string8);
  vector *lhs = VEC_CREATE(string8);
  string8 sep = STR8_LIT(" | ");

  u64 part1 = 0;
  u64 part2 = 0;
  u8 mapping[10];

  for (u64 i = 0; i < line_vec->size; ++i) {
    str_split_once(splitted, lines[i], sep);
    split_whitespace(rhs, splitted[1]);
    // Part 1
    string8 *nums = rhs->data;
    for (u64 j = 0; j < rhs->size; ++j) {
      if ((nums[j].size >= 2 && nums[j].size <= 4) || nums[j].size == 7)
        part1++;
    }
    // Part 2
    split_whitespace(lhs, splitted[0]);
    find_mapping(mapping, (string8 *)lhs->data);
    u64 decoded = decode_rhs(mapping, (string8 *)rhs->data);
    // printf(STR8_FMT " => %lu\n", STR8_UNWRAP(splitted[1]), decoded);
    part2 += decoded;
  }
  vector_free(rhs);
  vector_free(lhs);

  vector_free(line_vec);

  printf("Part 1: %lu\n", part1);
  printf("Part 2: %lu\n", part2);

  free(fc.str);
}
