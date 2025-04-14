#include "keypad.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  keypad_t *num_pad = create_num_keypad();
  keypad_t *arrow_pad = create_dir_keypad();
  cache_t *cache = cache_create();
  sequences_t *arrow_seqs = sequences_from_keypad(arrow_pad);

  FILE *file = fopen(argv[1], "r");
  if (!file) {
    fprintf(stderr, "Failed to open file: %s\n", argv[1]);
    return EXIT_FAILURE;
  }
  char buffer[16];
  vector_t *all_num_sequences;
  uint64_t complexity_sum = 0;
  while (fgets(buffer, 16, file)) {
    int n = strcspn(buffer, "\n");
    buffer[n] = '\0';
    all_num_sequences = keypad_get_sequences_from_keys(num_pad, buffer, n);
    struct node_t *node = all_num_sequences->head;
    uint64_t optimal = UINT64_MAX;
    while (node) {
      uint64_t length = 0;
      char key_from = 'A';
      char key_to;
      for (int i = 0; i < node->size; ++i) {
        char key_to = key_as_char(node->value[i]);
        length +=
            min_distance(arrow_pad, key_from, key_to, 25, cache, arrow_seqs);
        key_from = key_to;
      }
      node = node->next;
      if (length < optimal) {
        optimal = length;
      }
    }
    vector_free(all_num_sequences);
    printf("Optimal length for sequence %s: %lu\n", buffer, optimal);
    int num = 0;
    for (int i = 0; i < n - 1; ++i) {
      num = num * 10 + (buffer[i] - '0');
    }
    complexity_sum += optimal * (uint64_t)num;
  }
  printf("Complexity sum: %lu\n", complexity_sum);

  keypad_free(num_pad);
  cache_free(cache);
  sequences_free(arrow_seqs);

  return EXIT_SUCCESS;
}
