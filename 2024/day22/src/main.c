#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CAPACITY 7919

struct node {
  uint32_t encoded;
  int gain;
  struct node *next;
};

typedef struct {
  int capacity;
  int best;
  uint32_t best_sequence;
  struct node **arr;
} seq_map;

seq_map *create_sequence_map(int capacity) {
  seq_map *sm = malloc(sizeof(seq_map));
  sm->capacity = capacity;
  sm->best = 0;
  sm->arr = (struct node **)malloc(sizeof(struct node *) * capacity);
  for (int i = 0; i < capacity; ++i) {
    sm->arr[i] = NULL;
  }
  return sm;
}

struct node *create_node(uint32_t encoded, int gain) {
  struct node *seq = (struct node *)malloc(sizeof(struct node));
  seq->gain = gain;
  seq->encoded = encoded;
  seq->next = NULL;
  return seq;
}

void free_sequence_map(seq_map *sm) {
  struct node *seq;
  for (int i = 0; i < sm->capacity; ++i) {
    seq = sm->arr[i];
    while (seq != NULL) {
      struct node *next = seq->next;
      seq->next = NULL;
      free(seq);
      seq = next;
    }
    sm->arr[i] = NULL;
  }
  free(sm->arr);
  free(sm);
}

void insert_sequence(seq_map *sm, uint32_t encoded, int gain) {
  int index = encoded % sm->capacity;
  struct node *seq = create_node(encoded, gain);
  if (sm->arr[index] == NULL) {
    sm->arr[index] = seq;
    return;
  }
  seq->next = sm->arr[index];
  sm->arr[index] = seq;
}

struct node *get_sequence(seq_map *sm, uint32_t encoded) {
  int index = encoded % sm->capacity;
  struct node *seq = sm->arr[index];
  while (seq != NULL) {
    if (seq->encoded == encoded)
      return seq;
    seq = seq->next;
  }
  return NULL;
}

struct node *get_or_put(seq_map *sm, uint32_t key, int default_value) {
  int index = key % sm->capacity;
  struct node *seq = sm->arr[index];
  while (seq != NULL) {
    if (seq->encoded == key) {
      return seq;
    }
    seq = seq->next;
  }
  insert_sequence(sm, key, default_value);
  return sm->arr[index];
}

void add_sequence_map(seq_map *map_from, seq_map *map_to) {
  for (int i = 0; i < map_from->capacity; ++i) {
    struct node *seq = map_from->arr[i];
    while (seq != NULL) {
      struct node *next = seq->next;
      seq->next = NULL;
      // Add to other
      struct node *dst = get_or_put(map_to, seq->encoded, 0);
      dst->gain += seq->gain;
      if (dst->gain > map_to->best) {
        map_to->best = dst->gain;
        map_to->best_sequence = seq->encoded;
      }
      free(seq);
      seq = next;
    }
    map_from->arr[i] = NULL;
  }
}

void decode_sequence(uint32_t encoded, int *decoded) {
  for (int i = 0; i < 4; ++i) {
    decoded[3 - i] = (encoded & 0x1f) - 9;
    encoded = encoded >> 5;
  }
}

uint64_t mix(uint64_t x, uint64_t y) { return x ^ y; }

uint64_t prune(uint64_t x) { return x & ((1 << 24) - 1); }

uint64_t next_number(uint64_t x) {
  uint64_t result;
  result = prune(mix(x << 6, x));
  result = prune(mix(result >> 5, result));
  result = prune(mix(result << 11, result));
  return result;
}

int ones_digit(uint64_t x) { return (int)(x % 10); }

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  FILE *file = fopen(argv[1], "r");
  if (!file) {
    fprintf(stderr, "Could not open file '%s'\n", argv[1]);
    return EXIT_FAILURE;
  }
  char buffer[256];
  uint64_t secret;
  uint64_t part1 = 0;
  seq_map *sm = create_sequence_map(CAPACITY);
  seq_map *buyer = create_sequence_map(CAPACITY);
  int prev, price, num, ifound;
  int encoded;
  int counter = 0;
  while (fgets(buffer, 256, file)) {
    sscanf(buffer, "%lu", &secret);
    prev = ones_digit(secret);
    encoded = 0;
    for (int i = 0; i < 2000; i++) {
      secret = next_number(secret);
      price = ones_digit(secret);
      encoded = ((encoded << 5) + (price - prev + 9)) & 0xfffff;
      prev = price;
      if (i < 3)
        continue;
      get_or_put(buyer, encoded, price);
    }
    part1 += secret;
    add_sequence_map(buyer, sm);
  }
  fclose(file);

  int decoded[4];
  decode_sequence(sm->best_sequence, decoded);

  printf("Part 1: %lu\n", part1);
  printf("Part 2: best gain of %d with sequence (%d,%d,%d,%d)\n", sm->best,
         decoded[0], decoded[1], decoded[2], decoded[3]);

  free_sequence_map(sm);
  free_sequence_map(buyer);

  return EXIT_SUCCESS;
}
