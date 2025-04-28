#include "haunted_wasteland.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

pair_t *pair_create(char *lhs, char *rhs) {
  pair_t *pair = malloc(sizeof(pair_t));
  strncpy(pair->lhs, lhs, 4);
  strncpy(pair->rhs, rhs, 4);
  return pair;
}

hash_map_t *hash_map_create(int capacity, enum NodeType data_type) {
  hash_map_t *map = malloc(sizeof(hash_map_t));
  map->n_entry = 0;
  map->capacity = capacity;
  map->node_type = data_type;
  if (data_type == Pair) {
    map->table = malloc(sizeof(struct hm_pair_node_t *) * capacity);
  } else {
    map->table = malloc(sizeof(struct hm_uint_node_t *) * capacity);
  }
  for (int i = 0; i < capacity; ++i) {
    map->table[i] = NULL;
  }
  return map;
}

static int hash_key(int capacity, void *key, enum NodeType data_type) {
  uint64_t hash = FNV_OFFSET;
  switch (data_type) {
  case Pair:
    for (int i = 0; i < 3; ++i) {
      hash ^= (uint64_t)((char *)key)[i];
      hash *= FNV_PRIME;
    }
    break;
  case Uint:
    hash = (hash ^ *(uint64_t *)key) * FNV_PRIME;
    break;
  }
  return (int)(hash & (uint64_t)(capacity - 1));
}

static int compare_keys(struct hm_node_t *node, void *key,
                        enum NodeType node_type) {
  switch (node_type) {
  case Pair:
    return strncmp((char *)node->key, (char *)key, 4);
  case Uint:
    return *(uint64_t *)node->key - *(uint64_t *)key;
  }
  return 0;
}

void *hash_map_get(hash_map_t *map, void *key) {
  int index = hash_key(map->capacity, key, map->node_type);
  struct hm_node_t *node = map->table[index];
  while (node != NULL) {
    if (compare_keys(node, key, map->node_type) == 0)
      return node->value;
    node = node->next;
  }
  return NULL;
}

void hash_map_insert(hash_map_t *map, void *key, void *value) {
  struct hm_node_t *node;
  int index = hash_key(map->capacity, key, map->node_type);
  node = map->table[index];
  while (node) {
    if (compare_keys(node, key, map->node_type) == 0) {
      switch (map->node_type) {
      case Pair:
        free(node->value);
        node->value = value;
        return;
      case Uint:
        *(uint64_t *)node->value = *(uint64_t *)value;
        return;
      }
    }
    node = node->next;
  }
  node = malloc(sizeof(struct hm_node_t));
  switch (map->node_type) {
  case Pair:
    node->key = strdup((char *)key);
    node->value = value;
    break;
  case Uint:
    node->key = malloc(sizeof(uint64_t));
    *(uint64_t *)node->key = *(uint64_t *)key;
    node->value = malloc(sizeof(uint64_t));
    *(uint64_t *)node->value = *(uint64_t *)value;
    break;
  }
  node->next = map->table[index];
  map->table[index] = node;
  map->n_entry++;
}

void *hash_map_get_or_default(hash_map_t *map, void *key) {
  void *result = hash_map_get(map, key);
  if (result != NULL) {
    return result;
  }
  uint64_t default_value = 0;
  hash_map_insert(map, key, &default_value);
  return hash_map_get(map, key);
}

hash_map_t *hash_map_factorization(int value) {
  hash_map_t *map = hash_map_create(31, Uint);
  uint64_t value_ = (uint64_t)value;
  uint64_t d = 2;
  while (d * d <= value_) {
    while (value_ % d == 0) {
      uint64_t *count = hash_map_get_or_default(map, &d);
      (*count)++;
      value_ /= d;
    }
    d++;
  }
  if (value_ > 1) {
    uint64_t *count = hash_map_get_or_default(map, &value_);
    (*count)++;
  }
  for (int i = 0; i < map->capacity; ++i) {
  }
  return map;
}

void hash_map_free(hash_map_t *map) {
  for (int i = 0; i < map->capacity; ++i) {
    struct hm_node_t *current = map->table[i];
    while (current != NULL) {
      struct hm_node_t *next = current->next;
      free(current->key);
      free(current->value);
      free(current);
      current = next;
    }
  }
  free(map->table);
  free(map);
}

vector_t *vector_create() {
  vector_t *vec = malloc(sizeof(vector_t));
  vec->head = NULL;
  vec->tail = NULL;
  return vec;
}

void vector_push(vector_t *vec, char *data) {
  struct vec_node_t *node = malloc(sizeof(struct vec_node_t));
  strncpy(node->data, data, 4);
  node->next = NULL;
  if (vec->head == NULL) {
    vec->head = node;
    vec->tail = node;
  } else {
    vec->tail->next = node;
    vec->tail = node;
  }
}

void vector_free(vector_t *vec) {
  struct vec_node_t *node = vec->head;
  while (node != NULL) {
    struct vec_node_t *temp = node->next;
    free(node);
    node = temp;
  }
  free(vec);
}
