#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#include <inttypes.h>

enum Key { UP, RIGHT, DOWN, LEFT, A };

typedef struct {
  int n_keys, capacity;
  char *keys;
  int *keyX, *keyY;
  int nogoX, nogoY;
} keypad_t;

struct node_t {
  int size;
  int *value;
  struct node_t *next;
};

typedef struct {
  struct node_t *head;
  struct node_t *tail;
} vector_t;

typedef struct {
  keypad_t *keypad;
  vector_t **sequences;
} sequences_t;

typedef struct {
  char key_from, key_to;
  int depth;
} cache_key;

typedef struct {
  int size, capacity;
  cache_key *keys;
  uint64_t *values;
} cache_t;

vector_t *vector_create();
void vector_push(vector_t *vec, int *value, int size);
void vector_free(vector_t *vec);

keypad_t *keypad_create(int n, int nogoX, int nogoY);
int keypad_get_key_id(keypad_t *keypad, char key);
void keypad_free(keypad_t *keypad);
void keypad_add_key(keypad_t *keypad, char key, int x, int y);
void keypad_get_pos(keypad_t *keypad, char key, int *x, int *y);
void keypad_get_sequences_between_keys(keypad_t *keypad, int x_from, int y_from,
                                       char key_to, int *previous, int size,
                                       vector_t *motions);
vector_t *keypad_get_sequences_from_keys(keypad_t *keypad, char *keys,
                                         int n_keys);

void key_to_dx(enum Key, int *dx, int *dy);
char key_as_char(enum Key key);

keypad_t *create_num_keypad();
keypad_t *create_dir_keypad();

sequences_t *sequences_create(keypad_t *keypad);
vector_t *sequences_get(sequences_t *conn, char key_from, char key_to);
sequences_t *sequences_from_keypad(keypad_t *keypad);
void sequences_free(sequences_t *conn);

cache_t *cache_create();
int cache_get_index(cache_t *cache, char key_from, char key_to, int depth);
void cache_add(cache_t *cache, char key_from, char key_to, int depth,
               uint64_t value);
void cache_free(cache_t *cache);

uint64_t min_distance(keypad_t *keypad, char key_from, char key_to, int depth,
                      cache_t *cache, sequences_t *conn);

void print_sequence(int *seq, int size);
void print_vector(vector_t *vec);

#endif
