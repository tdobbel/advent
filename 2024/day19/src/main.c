#include <asm-generic/errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 5096
#define CAPACITY 10

struct node_t {
  char towel[CAPACITY];
  struct node_t *next;
};

typedef struct {
  int capacity, max_size;
  struct node_t **arr;
} towels_t;

towels_t *create_towels() {
  towels_t *towels = malloc(sizeof(towels_t));
  towels->capacity = CAPACITY;
  towels->max_size = 0;
  towels->arr = malloc(sizeof(struct node_t *) * towels->capacity);
  for (int i = 0; i < towels->capacity; ++i) {
    towels->arr[i] = NULL;
  }
  return towels;
}

struct node_t *create_node(const char *towel) {
  struct node_t *new_node = malloc(sizeof(struct node_t));
  strncpy(new_node->towel, towel, CAPACITY);
  new_node->next = NULL;
  return new_node;
}

void insert_towel(towels_t *towels, const char *towel, int size) {
  int index = size - 1;
  if (index >= towels->capacity) {
    perror("Index out of bounds");
    return;
  }
  if (size > towels->max_size) {
    towels->max_size = size;
  }
  struct node_t *node = create_node(towel);
  if (towels->arr[index] == NULL) {
    towels->arr[index] = node;
  } else {
    struct node_t *next = towels->arr[index];
    node->next = next;
    towels->arr[index] = node;
  }
}

int contains(const towels_t *towels, const char *towel, int size) {
  int index = size - 1;
  struct node_t *node = towels->arr[index];
  while (node) {
    if (strncmp(node->towel, towel, size) == 0)
      return 1;
    node = node->next;
  }
  return 0;
}

void free_towels(towels_t *towels) {
  for (int i = 0; i < towels->capacity; ++i) {
    struct node_t *current = towels->arr[i];
    while (current) {
      struct node_t *temp = current;
      current = current->next;
      temp->next = NULL;
      free(temp);
    }
  }
  free(towels->arr);
  free(towels);
}

uint64_t count_possibilities(const towels_t *towels, const char *design,
                             int size) {
  uint64_t *count = calloc(size, sizeof(uint64_t));
  char test[towels->max_size + 1];
  for (int i = 0; i < size; ++i) {
    int stop = i + 1;
    for (int j = 0; j < towels->max_size && j <= i; ++j) {
      int start = i - j;
      int n = j + 1;
      strncpy(test, design + start, n);
      test[n] = '\0';
      if (!contains(towels, test, n))
        continue;
      if (start > 0) {
        count[i] += count[start - 1];
      } else {
        count[i]++;
      }
    }
  }
  uint64_t total = count[size - 1];
  free(count);
  return total;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  FILE *file = fopen(argv[1], "r");
  if (!file) {
    perror("Error opening file");
    return EXIT_FAILURE;
  }

  char buffer[BUFFER_SIZE];

  // Read the first line and parse the towels
  char *ptr = fgets(buffer, BUFFER_SIZE, file);
  if (ptr == NULL) {
    perror("Could not parse towels");
    fclose(file);
    return EXIT_FAILURE;
  }
  int n = strcspn(buffer, "\n");
  char towel[CAPACITY];
  towels_t *towels = create_towels();
  int k = 0;
  for (int i = 0; i < n; ++i) {
    if (buffer[i] >= 'a' && buffer[i] <= 'z')
      towel[k++] = buffer[i];
    if (buffer[i] == ',' || i == n - 1) {
      towel[k] = '\0';
      insert_towel(towels, towel, k);
      k = 0;
    }
  }

  // Check possible patterns
  int part1 = 0;
  uint64_t part2 = 0;
  while (fgets(buffer, BUFFER_SIZE, file)) {
    n = strcspn(buffer, "\n");
    if (n == 0)
      continue;
    uint64_t n_sol = count_possibilities(towels, buffer, n);
    if (n_sol > 0) {
      part1++;
      part2 += n_sol;
    }
  }

  fclose(file);
  free_towels(towels);

  printf("Part 1: %d possible designs\n", part1);
  printf("Part 2: %lu ways to obtain these designs\n", part2);

  return EXIT_SUCCESS;
}
