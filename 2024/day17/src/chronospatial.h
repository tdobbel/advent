#ifndef _CHRONO_SPATIAL_
#define _CHRONO_SPATIAL_

#include <stdint.h>

typedef struct {
  uint64_t a, b, c;
} Register;

struct node_t {
  int value;
  struct node_t *next;
};

typedef struct {
  struct node_t *head;
  struct node_t *tail;
} vector_t;

Register *register_create(uint64_t a, uint64_t b, uint64_t c);

vector_t *vector_create();
void vector_free(vector_t *vec);
void vector_push(vector_t *vec, int value);
void vector_print(vector_t *vec);

uint64_t register_get_combo(Register *reg, int literal);
void register_excute_program(Register *reg, const int *program, int *index,
                             vector_t *output);
int find_a(const int *program, int program_size, uint64_t prev, int cntr,
           uint64_t *a);

#endif
