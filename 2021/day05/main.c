#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define BUFSIZE 64

#define SIGN(a) (((a) == 0) ? 0 : (((a) > 0) ? 1 : -1))
#define ABS(a) (((a) >= 0) ? (a) : -(a))
#define MAX(a, b) (((a) >= (b)) ? (a) : (b))

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef size_t usize;
typedef int32_t i32;

typedef struct {
  u16 a[2];
  u16 b[2];
} Segment;

void parse_line(char *line, Segment *seg) {
  int end = strcspn(line, "\n");
  int stop = strcspn(line, " -> ");
  u16 v = 0;
  int i = 0, k = 0;
  for (i = 0; i <= stop; ++i) {
    if (line[i] == ',' || i == stop) {
      seg->a[k++] = v;
      v = 0;
      continue;
    }
    v = v * 10 + line[i] - '0';
  }
  k = 0;
  for (i = stop + 4; i <= end; ++i) {
    if (line[i] == ',' || i == end) {
      seg->b[k++] = v;
      v = 0;
      continue;
    }
    v = v * 10 + line[i] - '0';
  }
}

static inline u32 murmur_32_scramble(u32 k) {
  k *= 0xcc9e2d51;
  k = (k << 15) | (k >> 17);
  k *= 0x1b873593;
  return k;
}

// Implementation for little endian
u32 murmur3_32(const u8 *key, usize len, u32 seed) {
  u32 h = seed;
  u32 k;
  // Read in groups of 4
  for (size_t i = len >> 2; i; i--) {
    memcpy(&k, key, sizeof(u32));
    key += sizeof(u32);
    h ^= murmur_32_scramble(k);
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;
  }
  // Read the rest.
  k = 0;
  for (size_t i = len & 3; i; i--) {
    k <<= 8;
    k |= key[i - 1];
  }
  // A swap is *not* necessary here because the preceding loop already
  // places the low bytes in the low places according to whatever endianness
  // we use. Swaps only apply when the memory is copied in a chunk.
  h ^= murmur_32_scramble(k);
  // Finalize
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

struct Node {
  u32 key;
  u16 counter;
  struct Node *next;
};

typedef struct {
  usize capacity;
  struct Node **values;
} HashMap;

HashMap *hashmap_create(usize capacity) {
  HashMap *hm = malloc(sizeof(HashMap));
  hm->capacity = capacity;
  hm->values = (struct Node **)malloc(sizeof(struct Node *) * capacity);
  return hm;
}

void hashmap_deinit(HashMap *hm) {
  for (usize i = 0; i < hm->capacity; ++i) {
    struct Node *node = hm->values[i];
    while (node) {
      struct Node *next = node->next;
      free(node);
      node = next;
    }
  }
  free(hm->values);
  free(hm);
}

void hashmap_increment(HashMap *hm, u16 x, u16 y) {
  u32 key = x << 16 | y;
  u32 hash = murmur3_32((u8 *)&key, 4, 0x00000000);
  usize i = hash % hm->capacity;
  struct Node *node = hm->values[i];
  while (node) {
    if (node->key == key) {
      node->counter++;
      return;
    }
    node = node->next;
  }
  // We did not return so the key was not found
  struct Node *inserted = malloc(sizeof(struct Node));
  inserted->key = key;
  inserted->counter = 1;
  inserted->next = hm->values[i];
  hm->values[i] = inserted;
}

void increment_path(Segment *seg, HashMap *hm) {
  i32 dx = (i32)seg->b[0] - (i32)seg->a[0];
  i32 dy = (i32)seg->b[1] - (i32)seg->a[1];
  usize n = MAX(ABS(dx), ABS(dy));
  dx = SIGN(dx);
  dy = SIGN(dy);
  u16 x = seg->a[0], y = seg->a[1];
  for (usize i = 0; i <= n; ++i) {
    hashmap_increment(hm, x, y);
    x += dx;
    y += dy;
  }
}

u32 count_overlaps(HashMap *hm) {
  u32 sum = 0;
  for (usize i = 0; i < hm->capacity; ++i) {
    struct Node *node = hm->values[i];
    while (node) {
      if (node->counter > 1)
        sum++;
      node = node->next;
    }
  }
  return sum;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(argv[1], "r");
  if (!fp) {
    fprintf(stderr, "Could not open file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  char line[BUFSIZE];
  Segment *seg = malloc(sizeof(Segment));
  HashMap *hm1 = hashmap_create(1 << 15);
  HashMap *hm2 = hashmap_create(1 << 15);
  while (fgets(line, BUFSIZE, fp)) {
    parse_line(line, seg);
    increment_path(seg, hm2);
    if (seg->a[0] == seg->b[0] || seg->a[1] == seg->b[1]) {
      increment_path(seg, hm1);
    }
  }
  free(seg);
  fclose(fp);

  u32 part1 = count_overlaps(hm1);
  printf("Part1: %u\n", part1);
  hashmap_deinit(hm1);

  u32 part2 = count_overlaps(hm2);
  printf("Part2: %u\n", part2);
  hashmap_deinit(hm2);

  return EXIT_SUCCESS;
}
