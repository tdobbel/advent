#include "haunted_wasteland.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

int part1(hash_map_t *map, char *instructions, int n_instruction, char *start) {
  char *current = start;
  int cntr = 0;
  while (1) {
    if (current[2] == 'Z' && cntr > 0)
      break;
    pair_t *pair = hash_map_get(map, current);
    char instruction = instructions[cntr % n_instruction];
    if (instruction == 'R') {
      current = pair->rhs;
    } else {
      current = pair->lhs;
    }
    cntr++;
  }
  return cntr;
}

uint64_t part2(hash_map_t *map, vector_t *start_nodes, char *instructions,
               int n_instruction) {
  hash_map_t *factors = hash_map_create(53, Uint);
  struct vec_node_t *start = start_nodes->head;
  struct hm_node_t *entry;
  while (start) {
    int n_step = part1(map, instructions, n_instruction, start->data);
    hash_map_t *temp = hash_map_factorization(n_step);
    for (int i = 0; i < temp->capacity; ++i) {
      entry = temp->table[i];
      while (entry) {
        uint64_t *key = entry->key;
        uint64_t *value = entry->value;
        uint64_t *counter = hash_map_get_or_default(factors, key);
        if (*value > *counter) {
          *counter = *value;
        }
        entry = entry->next;
      }
    }
    hash_map_free(temp);
    start = start->next;
  }
  uint64_t result = 1;
  for (int i = 0; i < factors->capacity; ++i) {
    entry = factors->table[i];
    while (entry) {
      uint64_t *factor = entry->key;
      uint64_t *cnt = entry->value;
      result *= (uint64_t)pow((double)(*factor), (double)(*cnt));
      entry = entry->next;
    }
  }
  hash_map_free(factors);
  return result;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  FILE *file = fopen(argv[1], "r");
  if (!file) {
    perror("Failed to open file");
    return EXIT_FAILURE;
  }
  char buffer[BUFFER_SIZE];
  char instructions[BUFFER_SIZE];
  int first = 1;
  int n_instruction;
  char key[4], lhs[4], rhs[4];
  key[3] = '\0';
  lhs[3] = '\0';
  rhs[3] = '\0';
  hash_map_t *map = hash_map_create(7001, Pair);
  vector_t *start_nodes = vector_create();
  while (fgets(buffer, BUFFER_SIZE, file)) {
    int n = strcspn(buffer, "\n");
    if (first) {
      strncpy(instructions, buffer, n);
      instructions[n] = '\0';
      n_instruction = n;
      first = 0;
    }
    if (n == 0) {
      continue;
    }
    strncpy(key, buffer + 0, 3);
    strncpy(lhs, buffer + 7, 3);
    strncpy(rhs, buffer + 12, 3);
    pair_t *pair = pair_create(lhs, rhs);
    hash_map_insert(map, key, pair);
    if (key[2] == 'A') {
      vector_push(start_nodes, key);
    }
  }
  fclose(file);
  int sol1 = part1(map, instructions, n_instruction, "AAA");
  printf("Part 1: %d\n", sol1);
  uint64_t sol2 = part2(map, start_nodes, instructions, n_instruction);
  printf("Part 2: %lu\n", sol2);

  hash_map_free(map);
  vector_free(start_nodes);

  return EXIT_SUCCESS;
}
