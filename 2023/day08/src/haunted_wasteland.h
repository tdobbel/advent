#ifndef HAUNTED_WASTELAND_H
#define HAUNTED_WASTELAND_H

#include <stdint.h>

enum NodeType { Uint, Pair };

typedef struct {
  char lhs[4], rhs[4];
} pair_t;

struct hm_node_t {
  void *key;
  void *value;
  struct hm_node_t *next;
};

typedef struct {
  int capacity, n_entry;
  enum NodeType node_type;
  struct hm_node_t **table;
} hash_map_t;

struct vec_node_t {
  char data[4];
  struct vec_node_t *next;
};

typedef struct {
  struct vec_node_t *head;
  struct vec_node_t *tail;
} vector_t;

pair_t *pair_create(char *lhs, char *rhs);

hash_map_t *hash_map_create(int capacity, enum NodeType data_type);
void *hash_map_get(hash_map_t *map, void *key);
void hash_map_insert(hash_map_t *map, void *key, void *value);
void hash_map_free(hash_map_t *map);
void *hash_map_get_or_default(hash_map_t *map, void *key);
hash_map_t *hash_map_factorization(int value);

vector_t *vector_create();
void vector_push(vector_t *vec, char *data);
void vector_free(vector_t *vec);

#endif // HAUNTED_WASTELAND_H
