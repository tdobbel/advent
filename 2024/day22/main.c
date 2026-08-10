#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"
#define VECTOR_IMPLEMENTATION
#include "vector.h"
#define HASHMAP_IMPLEMENTATION
#include "hash_map.h"

#define SEQ_FMT "(%d, %d, %d, %d)"
#define SEQ_UNWRAP(s) (s)[0], (s)[1], (s)[2], (s)[3]

typedef int8_t i8;

inline u64 mix(u64 x, u64 y) { return x ^ y; }
inline u64 prune(u64 x) { return x & ((1 << 24) - 1); }

u32 encode_sequence(i8 seq[4]);
void decode_sequence(i8 seq[4], u32 encoded);
u64 next_number(u64 x);
void display_seq(hash_map *hm);

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

  hash_map *buyer = AUTO_HASHMAP(u32, u32);
  hash_map *total = AUTO_HASHMAP(u32, u32);

  u64 part1 = 0;
  u32 part2 = 0;
  i8 sequence[4] = {0};
  i8 best[4] = {0};

  for (u64 i = 0; i < line_vec->size && lines[i].size > 0; ++i) {
    u64 secret = str_parse_unsigned(lines[i]);
    i8 prev = secret % 10;
    for (u64 j = 0; j < 2000; ++j) {
      secret = next_number(secret);
      i8 price = secret % 10;
      for (u32 i = 0; i < 3; ++i) {
        sequence[i] = sequence[i + 1];
      }
      sequence[3] = price - prev;
      prev = price;
      if (j < 3)
        continue;
      u32 encoded = encode_sequence(sequence);
      kv_entry entry = hm_get_or_put(buyer, &encoded);
      if (!entry.found_existing) {
        *(u32 *)entry.value_ptr = (u32)price;
      }
    }
    kv_iterator kvi = hm_iterator(buyer);
    while (get_next(&kvi)) {
      kv_entry entry = hm_get_or_put(total, kvi.key_ptr);
      u32 *total_win = (u32 *)entry.value_ptr;
      if (!entry.found_existing) {
        *total_win = 0;
      }
      *total_win += *(u32 *)kvi.value_ptr;
      if (*total_win > part2) {
        part2 = *total_win;
        decode_sequence(best, *(u32 *)kvi.key_ptr);
      }
    }
    hm_reset(buyer);
    part1 += secret;
  }

  // display_seq(total);
  printf("Part 1: %lu\n", part1);
  printf("Part 2: %u with sequence " SEQ_FMT "\n", part2, SEQ_UNWRAP(best));

  vector_free(line_vec);
  hm_deinit(buyer);
  hm_deinit(total);
  free(file.str);

  return EXIT_SUCCESS;
}

u32 encode_sequence(i8 seq[4]) {
  u32 encoded = 0;
  for (u32 i = 0; i < 4; ++i) {
    encoded <<= 8;
    encoded |= (u8)(seq[i] + 9);
  }
  return encoded;
}

void decode_sequence(i8 seq[4], u32 encoded) {
  for (u32 i = 0; i < 4; ++i) {
    seq[3 - i] = (i8)(encoded & 0xff) - 9;
    encoded >>= 8;
  }
}

void display_seq(hash_map *hm) {
  kv_iterator kvi = hm_iterator(hm);
  i8 seq[4];
  while (get_next(&kvi)) {
    u32 encoded = *(u32 *)kvi.key_ptr;
    decode_sequence(seq, encoded);
    u32 win = *(u32 *)kvi.value_ptr;
    printf(SEQ_FMT " => %u\n", win, SEQ_UNWRAP(seq));
  }
}

u64 next_number(u64 x) {
  u64 result = prune(mix(x << 6, x));
  result = prune(mix(result >> 5, result));
  result = prune(mix(result << 11, result));
  return result;
}
