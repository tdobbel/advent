#include "libmaze.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

city_t *city_from_file(char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Failed to open file");
    return NULL;
  }
  char buffer[BUFFER_SIZE];
  city_t *city = malloc(sizeof(city_t));
  int nx, ny = 0;
  int *pixels = malloc(sizeof(int) * BUFFER_SIZE * BUFFER_SIZE);
  while (fgets(buffer, sizeof(buffer), file)) {
    nx = strcspn(buffer, "\n");
    for (int x = 0; x < nx; ++x) {
      pixels[ny * nx + x] = buffer[x] - '0';
    }
    ny++;
  }
  fclose(file);
  pixels = realloc(pixels, sizeof(int) * nx * ny);
  city->nx = nx;
  city->ny = ny;
  city->blocks = malloc(sizeof(int *) * ny);
  city->blocks[0] = pixels;
  for (int i = 1; i < ny; ++i) {
    city->blocks[i] = city->blocks[i - 1] + nx;
  }
  return city;
}

void city_free(city_t *city) {
  if (city) {
    free(city->blocks[0]);
    free(city->blocks);
    free(city);
  }
}

binary_heap_t *binary_heap_create() {
  binary_heap_t *bh = malloc(sizeof(binary_heap_t));
  bh->size = 0;
  bh->capacity = 15;
  bh->nodes = malloc(sizeof(heap_node_t) * bh->capacity);
  return bh;
}

void binary_heap_free(binary_heap_t *bh) {
  for (int i = 0; i < bh->size; ++i) {
    free(bh->nodes[i].state);
  }
  free(bh->nodes);
  free(bh);
}

static void binary_heap_swap(heap_node_t *a, heap_node_t *b) {
  heap_node_t temp = *a;
  *a = *b;
  *b = temp;
}

static void heapify(binary_heap_t *bh, int index) {
  int left = 2 * index + 1;
  int right = 2 * index + 2;
  int smallest = index;
  if (left < bh->size &&
      bh->nodes[left].priority < bh->nodes[smallest].priority) {
    smallest = left;
  }
  if (right < bh->size &&
      bh->nodes[right].priority < bh->nodes[smallest].priority) {
    smallest = right;
  }
  if (smallest != index) {
    binary_heap_swap(bh->nodes + index, bh->nodes + smallest);
    heapify(bh, smallest);
  }
}

void binary_heap_delete(binary_heap_t *bh, int index) {
  if (index < 0 || index >= bh->size) {
    return;
  }
  bh->nodes[index] = bh->nodes[bh->size - 1];
  bh->size--;
  heapify(bh, index);
}

state_t *binary_heap_pop(binary_heap_t *bh) {
  if (bh->size == 0)
    return NULL;
  state_t *state = bh->nodes[0].state;
  binary_heap_delete(bh, 0);
  return state;
}

void binary_heap_insert(binary_heap_t *bh, state_t *state, int priority) {
  if (bh->size == bh->capacity) {
    bh->capacity *= 2;
    bh->nodes = realloc(bh->nodes, sizeof(heap_node_t) * bh->capacity);
  }
  heap_node_t *node = bh->nodes + bh->size;
  node->state = state;
  node->priority = priority;
  bh->size++;
  int i = bh->size - 1;
  while (i > 0) {
    if (bh->nodes[(i - 1) / 2].priority <= bh->nodes[i].priority)
      break;
    binary_heap_swap(bh->nodes + i, bh->nodes + (i - 1) / 2);
    i = (i - 1) / 2;
  }
}

hash_set_t *hash_set_create(int capacity) {
  hash_set_t *self = malloc(sizeof(hash_set_t));
  self->capacity = capacity;
  self->table = malloc(sizeof(struct hash_set_node_t *) * capacity);
  for (int i = 0; i < capacity; ++i) {
    self->table[i] = NULL;
  }
  return self;
}

void hash_set_free(hash_set_t *set) {
  for (int i = 0; i < set->capacity; ++i) {
    struct hash_set_node_t *node = set->table[i];
    while (node) {
      struct hash_set_node_t *temp = node;
      node = node->next;
      free(temp);
    }
  }
  free(set->table);
  free(set);
}

static int hash_key(int capacity, int x, int y, int counter,
                    enum Direction dir) {
  int values[4] = {x, y, counter, dir};
  uint64_t hash = FNV_OFFSET;
  for (int i = 0; i < 4; ++i) {
    hash ^= values[i];
    hash *= FNV_PRIME;
  }
  return (int)(hash & (uint64_t)(capacity - 1));
}

int hash_set_insert(hash_set_t *set, int x, int y, int counter,
                    enum Direction dir) {
  int index = hash_key(set->capacity, x, y, counter, dir);
  struct hash_set_node_t *node = set->table[index];
  struct hash_set_node_t *prev = NULL;
  while (node) {
    if (node->x == x && node->y == y && node->counter == counter &&
        node->dir == dir) {
      return 0; // Already exists
    }
    prev = node;
    node = node->next;
  }
  struct hash_set_node_t *new_node = malloc(sizeof(struct hash_set_node_t));
  new_node->x = x;
  new_node->y = y;
  new_node->counter = counter;
  new_node->dir = dir;
  new_node->next = NULL;
  if (prev) {
    prev->next = new_node;
  } else {
    set->table[index] = new_node;
  }
  return 1; // Insert new node
}

static void dir_to_dx(enum Direction dir, int *dx, int *dy) {
  switch (dir) {
  case UP:
    *dx = 0;
    *dy = -1;
    break;
  case RIGHT:
    *dx = 1;
    *dy = 0;
    break;
  case DOWN:
    *dx = 0;
    *dy = 1;
    break;
  case LEFT:
    *dx = -1;
    *dy = 0;
    break;
  }
}

static int distance(int x1, int y1, int x2, int y2) {
  return abs(x1 - x2) + abs(y1 - y2);
}

static void add_next_states(state_t *current, city_t *city, hash_set_t *visited,
                            binary_heap_t *queue, int min_blocks,
                            int max_blocks) {
  int dx, dy;
  int next_dir[3] = {(current->dir + 3) % 4, current->dir,
                     (current->dir + 1) % 4};
  for (int i = 0; i < 3; ++i) {
    dir_to_dx(next_dir[i], &dx, &dy);
    int x_next = current->x + dx;
    int y_next = current->y + dy;
    if (x_next < 0 || x_next >= city->nx || y_next < 0 || y_next >= city->ny) {
      continue;
    }
    if (current->counter > 0 && current->counter < min_blocks &&
        next_dir[i] != current->dir) {
      continue;
    }
    int cntr = next_dir[i] == current->dir ? current->counter + 1 : 1;
    if (cntr > max_blocks) {
      continue;
    }
    if (hash_set_insert(visited, x_next, y_next, cntr, next_dir[i]) == 0) {
      continue;
    }
    state_t *next = malloc(sizeof(state_t));
    next->x = x_next;
    next->y = y_next;
    next->dir = next_dir[i];
    next->counter = cntr;
    next->score = current->score + city->blocks[y_next][x_next];
    int priority =
        distance(x_next, y_next, city->nx - 1, city->ny - 1) + next->score;
    binary_heap_insert(queue, next, priority);
  }
}

int find_shortest_path(city_t *city, int min_blocks, int max_blocks) {
  int x_end = city->nx - 1;
  int y_end = city->ny - 1;
  state_t *start = malloc(sizeof(state_t));
  start->x = 0;
  start->y = 0;
  start->dir = RIGHT;
  start->counter = 0;
  start->score = 0;
  binary_heap_t *queue = binary_heap_create();
  binary_heap_insert(queue, start, distance(0, 0, x_end, y_end));
  hash_set_t *visited = hash_set_create(7919);
  int result = -1;
  while (queue->size > 0) {
    state_t *state = binary_heap_pop(queue);
    hash_set_insert(visited, state->x, state->y, state->counter, state->dir);
    if (state->x == x_end && state->y == y_end) {
      result = state->score;
      free(state);
      break;
    }
    add_next_states(state, city, visited, queue, min_blocks, max_blocks);
    free(state);
  }
  binary_heap_free(queue);
  hash_set_free(visited);
  return result;
}
