#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CAPACITY 2039

struct node {
  int numbers[4];
  int gain;
  struct node *next;
};

typedef struct {
  int capacity;
  int best;
  int best_seq[4];
  struct node **arr;
} seq_map;

int same_sequence(int x[4], int y[4]) {
  for (int i = 0; i < 4; ++i) {
    if (x[i] != y[i])
      return 0;
  }
  return 1;
}

int hash_sequence(seq_map *sm, int num[4]) {
  int index = 0, factor = 31;
  for (int i = 0; i < 4; ++i) {
    index = (31 * index + (num[i] + 9)) % sm->capacity;
  }
  return index;
}

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

struct node *create_node(int num[4], int gain) {
  struct node *seq = (struct node *)malloc(sizeof(struct node));
  seq->gain = gain;
  for (int i = 0; i < 4; ++i) {
    seq->numbers[i] = num[i];
  }
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

void insert_sequence(seq_map *sm, int num[4], int gain) {
  int index = hash_sequence(sm, num);
  struct node *seq = create_node(num, gain);
  if (sm->arr[index] == NULL) {
    sm->arr[index] = seq;
    return;
  }
  seq->next = sm->arr[index];
  sm->arr[index] = seq;
}

struct node *get_sequence(seq_map *sm, int num[4]) {
  int index = hash_sequence(sm, num);
  struct node *seq = sm->arr[index];
  while (seq != NULL) {
    if (same_sequence(seq->numbers, num))
      return seq;
    seq = seq->next;
  }
  return NULL;
}

void add_sequence_map(seq_map *sfrom, seq_map *sto) {
  for (int i = 0; i < sfrom->capacity; ++i) {
    struct node *seq = sfrom->arr[i];
    while (seq != NULL) {
      struct node *next = seq->next;
      seq->next = NULL;
      // Add to other
      struct node *seq_to = get_sequence(sto, seq->numbers);
      if (seq_to == NULL) {
        insert_sequence(sto, seq->numbers, 0);
        seq_to = get_sequence(sto, seq->numbers);
      }
      seq_to->gain += seq->gain;
      if (seq_to->gain > sto->best) {
        sto->best = seq_to->gain;
        for (int k = 0; k < 4; ++k) {
          sto->best_seq[k] = seq_to->numbers[k];
        }
      }
      seq = next;
    }
    sfrom->arr[i] = NULL;
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
  int prev, curr, ifound;
  int num[4] = {0, 0, 0, 0};
  int counter = 0;
  while (fgets(buffer, 256, file)) {
    sscanf(buffer, "%lu", &secret);
    prev = ones_digit(secret);
    for (int i = 0; i < 3; i++) {
      secret = next_number(secret);
      curr = ones_digit(secret);
      num[i] = curr - prev;
      prev = curr;
    }
    for (int i = 3; i < 2000; ++i) {
      secret = next_number(secret);
      curr = ones_digit(secret);
      num[3] = curr - prev;
      prev = curr;
      if (get_sequence(buyer, num) == NULL) {
        insert_sequence(buyer, num, curr);
      }
      for (int j = 0; j < 3; ++j) {
        num[j] = num[j + 1];
      }
    }
    part1 += secret;
    add_sequence_map(buyer, sm);
  }
  fclose(file);

  printf("Part 1: %lu\n", part1);
  printf("Part 2: best gain of %d with sequence (%d,%d,%d,%d)\n", sm->best,
         sm->best_seq[0], sm->best_seq[1], sm->best_seq[2], sm->best_seq[3]);

  free_sequence_map(sm);
  free_sequence_map(buyer);

  return EXIT_SUCCESS;
}
