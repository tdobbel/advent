#ifndef _MAZE_H_
#define _MAZE_H_

enum Direction { UP, RIGHT, DOWN, LEFT };

typedef struct {
  int x, y;
  enum Direction dir;
  int counter;
  int score;
} state_t;

typedef struct {
  int nx, ny;
  int **blocks;
} city_t;

struct hash_set_node_t {
  int x, y, counter;
  enum Direction dir;
  struct hash_set_node_t *next;
};

typedef struct {
  int capacity;
  struct hash_set_node_t **table;
} hash_set_t;

typedef struct {
  state_t *state;
  int priority;
} heap_node_t;

typedef struct {
  int size, capacity;
  heap_node_t *nodes;
} binary_heap_t;

city_t *city_from_file(char *filename);
void city_free(city_t *city);

binary_heap_t *binary_head_create();
void binary_heap_free(binary_heap_t *bh);
void binary_heap_insert(binary_heap_t *bh, state_t *state, int priority);
void binary_heap_delete(binary_heap_t *bh, int index);
state_t *binary_heap_pop(binary_heap_t *bh);

hash_set_t *hash_set_create(int capacity);
void hash_set_free(hash_set_t *set);
int hash_set_insert(hash_set_t *set, int x, int y, int counter,
                    enum Direction dir);

int find_shortest_path(city_t *city, int min_blocks, int max_blocks);

#endif
