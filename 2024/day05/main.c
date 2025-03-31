#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256
#define MAX_LIST_SIZE 64

typedef struct {
  int size, capacity;
  int value;
  int *larger;
} Ordering;

typedef struct {
  int size, capacity;
  Ordering *orderings;
} OrderingList;

void addLarger(Ordering *self, int larger) {
  if (self->size == self->capacity) {
    self->capacity *= 2;
    self->larger = (int *)realloc(self->larger, self->capacity * sizeof(int));
  }
  self->larger[self->size++] = larger;
}

OrderingList *createOrderingList() {
  OrderingList *self = (OrderingList *)malloc(sizeof(OrderingList));
  self->size = 0;
  self->capacity = 512;
  self->orderings = (Ordering *)malloc(self->capacity * sizeof(Ordering));
  return self;
}

void addOrdering(OrderingList *self, int low, int high) {
  Ordering *ordering;
  for (int i = 0; i < self->size; ++i) {
    ordering = &self->orderings[i];
    if (ordering->value == low) {
      addLarger(ordering, high);
      return;
    }
  }
  if (self->size == self->capacity) {
    self->capacity *= 2;
    self->orderings =
        (Ordering *)realloc(self->orderings, self->capacity * sizeof(Ordering));
  }
  ordering = &self->orderings[self->size++];
  ordering->size = 0;
  ordering->capacity = 32;
  ordering->value = low;
  ordering->larger = (int *)malloc(ordering->capacity * sizeof(int));
  addLarger(ordering, high);
}

void freeOrderingList(OrderingList *self) {
  for (int i = 0; i < self->size; ++i) {
    free(self->orderings[i].larger);
  }
  free(self->orderings);
  free(self);
}

int *getLarger(OrderingList *orders, int value, int *size) {
  for (int i = 0; i < orders->size; ++i) {
    if (orders->orderings[i].value == value) {
      *size = orders->orderings[i].size;
      return orders->orderings[i].larger;
    }
  }
  return NULL;
}

void parseList(char *buffer, int *numbers, int *size) {
  int n = strcspn(buffer, "\n");
  int index = 0;
  int value = 0;
  for (int i = 0; i < n; ++i) {
    if (buffer[i] == ',') {
      numbers[index++] = value;
      value = 0;
    } else {
      assert(buffer[i] >= '0' && buffer[i] <= '9');
      value = value * 10 + buffer[i] - '0';
    }
  }
  numbers[index++] = value;
  *size = index;
  assert(*size <= MAX_LIST_SIZE);
}

int isLarger(OrderingList *orders, int ref, int test) {
  int n;
  int *larger = getLarger(orders, ref, &n);
  if (larger == NULL)
    return 0;
  for (int i = 0; i < n; ++i) {
    if (larger[i] == test)
      return 1;
  }
  return 0;
}

int correctOrder(OrderingList *orders, int *numbers, int n_numbers) {
  int n;
  for (int i = n_numbers - 1; i > 0; --i) {
    for (int j = i - 1; j >= 0; --j) {
      if (isLarger(orders, numbers[i], numbers[j]))
        return 0;
    }
  }
  return 1;
}

int compareWithOrdering(const void *a, const void *b, void *orders) {
  int *ia = (int *)a;
  int *ib = (int *)b;
  OrderingList *orderings = (OrderingList *)orders;
  if (isLarger(orderings, *ia, *ib))
    return -1;
  if (isLarger(orderings, *ib, *ia))
    return 1;
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return 1;
  }

  int left, right;
  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    printf("Error opening file\n");
    return 1;
  }

  char buffer[BUFFER_SIZE];
  int numbers[MAX_LIST_SIZE];
  int n_numbers;
  int ispair = 1;
  int part1 = 0, part2 = 0;
  OrderingList *orders = createOrderingList();

  while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
    int n = strcspn(buffer, "\n");
    if (n == 0) {
      ispair = 0;
      continue;
    }
    if (ispair) {
      sscanf(buffer, "%d|%d", &left, &right);
      addOrdering(orders, left, right);
    } else {
      parseList(buffer, numbers, &n_numbers);
      if (correctOrder(orders, numbers, n_numbers)) {
        part1 += numbers[n_numbers / 2];
      } else {
        qsort_r(numbers, n_numbers, sizeof(int), compareWithOrdering, orders);
        part2 += numbers[n_numbers / 2];
      }
    }
  }
  fclose(file);

  printf("Part 1: %d\n", part1);
  printf("Part 2: %d\n", part2);

  freeOrderingList(orders);
  return 0;
}
