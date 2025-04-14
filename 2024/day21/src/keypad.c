#include "keypad.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

keypad_t *keypad_create(int n, int nogoX, int nogoY) {
  keypad_t *keypad = malloc(sizeof(keypad_t));
  keypad->n_keys = 0;
  keypad->capacity = n;
  keypad->keys = malloc(n * sizeof(char));
  keypad->keyX = malloc(n * sizeof(int));
  keypad->keyY = malloc(n * sizeof(int));
  keypad->nogoX = nogoX;
  keypad->nogoY = nogoY;
  return keypad;
}

void keypad_free(keypad_t *keypad) {
  free(keypad->keys);
  free(keypad->keyX);
  free(keypad->keyY);
  free(keypad);
}

void keypad_add_key(keypad_t *keypad, char key, int x, int y) {
  if (keypad->n_keys >= keypad->capacity) {
    fprintf(stderr, "Keypad capacity exceeded\n");
    exit(EXIT_FAILURE);
  }
  keypad->keys[keypad->n_keys] = key;
  keypad->keyX[keypad->n_keys] = x;
  keypad->keyY[keypad->n_keys] = y;
  keypad->n_keys++;
}

int keypad_get_key_id(keypad_t *keypad, char key) {
  for (int i = 0; i < keypad->n_keys; ++i) {
    if (keypad->keys[i] == key)
      return i;
  }
  fprintf(stderr, "Key not found :(\n");
  return -1;
}

void key_to_dx(enum Key key, int *dx, int *dy) {
  *dx = 0;
  *dy = 0;
  switch (key) {
  case UP:
    *dy = -1;
    break;
  case LEFT:
    *dx = -1;
    break;
  case DOWN:
    *dy = 1;
    break;
  case RIGHT:
    *dx = 1;
    break;
  case A:
    fprintf(stderr, "This should not be happening\n");
    exit(EXIT_FAILURE);
  }
}

char key_as_char(enum Key key) {
  switch (key) {
  case UP:
    return '^';
  case LEFT:
    return '<';
  case DOWN:
    return 'v';
  case RIGHT:
    return '>';
  case A:
    return 'A';
  }
}

void print_sequence(int *values, int size) {
  for (int i = 0; i < size; ++i) {
    printf("%c", key_as_char(values[i]));
  }
  printf("\n");
}

void print_vector(vector_t *vec) {
  struct node_t *node = vec->head;
  while (node) {
    print_sequence(node->value, node->size);
    node = node->next;
  }
}

keypad_t *create_num_keypad() {
  keypad_t *keypad = keypad_create(11, 0, 3);
  keypad_add_key(keypad, '0', 1, 3);
  keypad_add_key(keypad, 'A', 2, 3);
  keypad_add_key(keypad, '1', 0, 2);
  keypad_add_key(keypad, '2', 1, 2);
  keypad_add_key(keypad, '3', 2, 2);
  keypad_add_key(keypad, '4', 0, 1);
  keypad_add_key(keypad, '5', 1, 1);
  keypad_add_key(keypad, '6', 2, 1);
  keypad_add_key(keypad, '7', 0, 0);
  keypad_add_key(keypad, '8', 1, 0);
  keypad_add_key(keypad, '9', 2, 0);
  return keypad;
}

keypad_t *create_dir_keypad() {
  keypad_t *keypad = keypad_create(5, 0, 0);
  keypad_add_key(keypad, '^', 1, 0);
  keypad_add_key(keypad, 'A', 2, 0);
  keypad_add_key(keypad, '<', 0, 1);
  keypad_add_key(keypad, 'v', 1, 1);
  keypad_add_key(keypad, '>', 2, 1);
  return keypad;
}

void keypad_get_pos(keypad_t *keypad, char key, int *x, int *y) {
  for (int i = 0; i < keypad->n_keys; ++i) {
    if (key == keypad->keys[i]) {
      *x = keypad->keyX[i];
      *y = keypad->keyY[i];
      return;
    }
  }
  fprintf(stderr, "Key not found !\n");
  exit(EXIT_FAILURE);
}

vector_t *vector_create() {
  vector_t *vec = malloc(sizeof(vector_t));
  vec->head = NULL;
  vec->tail = NULL;
  return vec;
}

void vector_free(vector_t *vec) {
  struct node_t *node = vec->head;
  vec->head = NULL;
  while (node) {
    struct node_t *next = node->next;
    node->next = NULL;
    free(node->value);
    free(node);
    node = next;
  }
  vec->tail = NULL;
  free(vec);
}

void vector_push(vector_t *vec, int *value, int size) {
  struct node_t *node = malloc(sizeof(struct node_t));
  node->value = value;
  node->size = size;
  node->next = NULL;
  if (vec->head == NULL)
    vec->head = node;
  struct node_t *tail = vec->tail;
  if (tail != NULL) {
    tail->next = node;
  }
  vec->tail = node;
  tail = NULL;
}

static int *append_motion(int *motions, int size, enum Key extra_motion) {
  int *new_motions = (int *)malloc(sizeof(int) * (size + 1));
  for (int i = 0; i < size; ++i) {
    new_motions[i] = motions[i];
  }
  new_motions[size] = extra_motion;
  return new_motions;
}

void keypad_get_sequences_between_keys(keypad_t *keypad, int x_from, int y_from,
                                       char key_to, int *previous, int size,
                                       vector_t *motions) {
  int x_to, y_to;
  keypad_get_pos(keypad, key_to, &x_to, &y_to);
  if (x_from == x_to && y_from == y_to) {
    int *new_motions = append_motion(previous, size, A);
    vector_push(motions, new_motions, size + 1);
  } else {
    int dx_ = x_to - x_from;
    int dy_ = y_to - y_from;
    int dx, dy;
    for (int k = 0; k < 4; ++k) {
      key_to_dx(k, &dx, &dy);
      if (dx_ * dx == 0 && dy_ * dy == 0)
        continue;
      if (dx_ * dx < 0 || dy_ * dy < 0)
        continue;
      int x_new = x_from + dx;
      int y_new = y_from + dy;

      if (x_new == keypad->nogoX && y_new == keypad->nogoY)
        continue;
      int *new_keys = append_motion(previous, size, k);
      keypad_get_sequences_between_keys(keypad, x_new, y_new, key_to, new_keys,
                                        size + 1, motions);
    }
  }
  if (previous != NULL) {
    free(previous);
  }
}

static int *concatenate_sequences(int *arr1, int size1, int *arr2, int size2) {
  int *concat = (int *)malloc(sizeof(int) * (size1 + size2));
  for (int i = 0; i < size1; ++i) {
    concat[i] = arr1[i];
  }
  for (int i = 0; i < size2; ++i) {
    concat[i + size1] = arr2[i];
  }
  return concat;
}

vector_t *keypad_get_sequences_from_keys(keypad_t *keypad, char *keys,
                                         int n_keys) {
  int x_from = 2, y_from = 3;
  vector_t *sequences = vector_create();
  for (int i = 0; i < n_keys; ++i) {
    char key_to = keys[i];
    vector_t *next = vector_create();
    keypad_get_sequences_between_keys(keypad, x_from, y_from, key_to, NULL, 0,
                                      next);
    if (sequences->head == NULL) {
      vector_free(sequences);
      sequences = next;
      next = NULL;
    } else {
      vector_t *new_sequences = vector_create();
      struct node_t *node0 = sequences->head;
      while (node0) {
        struct node_t *node1 = next->head;
        while (node1) {
          int *value = concatenate_sequences(node0->value, node0->size,
                                             node1->value, node1->size);
          vector_push(new_sequences, value, node0->size + node1->size);
          node1 = node1->next;
        }
        node0 = node0->next;
      }
      vector_free(next);
      vector_free(sequences);
      sequences = new_sequences;
      new_sequences = NULL;
    }
    keypad_get_pos(keypad, key_to, &x_from, &y_from);
  }
  return sequences;
}

sequences_t *sequences_create(keypad_t *keypad) {
  sequences_t *conn = malloc(sizeof(sequences_t));
  conn->keypad = keypad;
  conn->sequences =
      malloc(sizeof(vector_t *) * keypad->n_keys * keypad->n_keys);
  return conn;
}
void sequences_free(sequences_t *conn) {
  int nk = conn->keypad->n_keys;
  keypad_free(conn->keypad);
  for (int i = 0; i < nk * nk; ++i) {
    vector_t *seq = conn->sequences[i];
    if (seq != NULL) {
      vector_free(seq);
    }
  }
  free(conn->sequences);
  free(conn);
}

vector_t *sequences_get(sequences_t *conn, char key_from, char key_to) {
  int nk = conn->keypad->n_keys;
  int i = keypad_get_key_id(conn->keypad, key_from);
  int j = keypad_get_key_id(conn->keypad, key_to);
  return conn->sequences[i * nk + j];
}

sequences_t *sequences_from_keypad(keypad_t *keypad) {
  sequences_t *conn = sequences_create(keypad);
  int nk = keypad->n_keys;
  for (int i = 0; i < keypad->n_keys; ++i) {
    int x_from = keypad->keyX[i];
    int y_from = keypad->keyY[i];
    for (int j = 0; j < keypad->n_keys; ++j) {
      char key_to = keypad->keys[j];
      vector_t *moves = vector_create();
      keypad_get_sequences_between_keys(keypad, x_from, y_from, key_to, NULL, 0,
                                        moves);
      conn->sequences[i * nk + j] = moves;
    }
  }
  return conn;
}

cache_t *cache_create() {
  cache_t *cache = malloc(sizeof(cache_t));
  cache->capacity = 1024;
  cache->size = 0;
  cache->keys = malloc(sizeof(cache_key) * cache->capacity);
  cache->values = malloc(sizeof(uint64_t) * cache->capacity);
  return cache;
}

int cache_get_index(cache_t *cache, char key_from, char key_to, int depth) {
  cache_key *ck;
  for (int i = 0; i < cache->size; ++i) {
    ck = &cache->keys[i];
    if (ck->key_from == key_from && ck->key_to == key_to && ck->depth == depth)
      return i;
  }
  return -1;
}

void cache_add(cache_t *cache, char key_from, char key_to, int depth,
               uint64_t value) {
  if (cache->size == cache->capacity) {
    cache->capacity *= 2;
    cache->keys = realloc(cache->keys, sizeof(cache_key) * cache->capacity);
    cache->values = realloc(cache->values, sizeof(uint64_t) * cache->capacity);
  }
  cache_key *ck = &cache->keys[cache->size];
  ck->depth = depth;
  ck->key_from = key_from;
  ck->key_to = key_to;
  cache->values[cache->size] = value;
  cache->size++;
}

void cache_free(cache_t *cache) {
  free(cache->keys);
  free(cache->values);
  free(cache);
}

uint64_t min_distance(keypad_t *keypad, char key_from, char key_to, int depth,
                      cache_t *cache, sequences_t *conn) {
  if (depth == 1) {
    int x_from, y_from, x_to, y_to;
    keypad_get_pos(keypad, key_from, &x_from, &y_from);
    keypad_get_pos(keypad, key_to, &x_to, &y_to);
    return (uint64_t)(abs(x_to - x_from) + abs(y_to - y_from) + 1);
  }
  vector_t *seqs = sequences_get(conn, key_from, key_to);
  uint64_t shortest = UINT64_MAX;
  struct node_t *node = seqs->head;
  while (node) {
    char kfrom = 'A';
    uint64_t path_length = 0;
    for (int i = 0; i < node->size; ++i) {
      char kto = key_as_char(node->value[i]);
      int index = cache_get_index(cache, kfrom, kto, depth - 1);
      uint64_t distance;
      if (index < 0) {
        distance = min_distance(keypad, kfrom, kto, depth - 1, cache, conn);
        cache_add(cache, kfrom, kto, depth - 1, distance);
      } else {
        distance = cache->values[index];
      }
      path_length += distance;
      kfrom = kto;
    }
    if (path_length < shortest)
      shortest = path_length;
    node = node->next;
  }
  return shortest;
}
