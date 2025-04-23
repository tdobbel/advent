#include "chronospatial.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

Register *register_create(uint64_t a, uint64_t b, uint64_t c) {
  Register *reg = malloc(sizeof(Register));
  reg->a = a;
  reg->b = b;
  reg->c = c;
  return reg;
};

uint64_t register_get_combo(Register *reg, int literal) {
  if (literal < 4)
    return (uint64_t)literal;
  if (literal == 4)
    return reg->a;
  if (literal == 5)
    return reg->b;
  if (literal == 6)
    return reg->c;
  perror("Not supposed to be here");
  exit(EXIT_FAILURE);
}

vector_t *vector_create() {
  vector_t *vec = malloc(sizeof(vector_t));
  vec->head = NULL;
  vec->tail = NULL;
  return vec;
}

void vector_free(vector_t *vec) {
  struct node_t *current = vec->head;
  while (current != NULL) {
    struct node_t *next = current->next;
    free(current);
    current = next;
  }
  free(vec);
}

void vector_push(vector_t *vec, int value) {
  struct node_t *new_node = malloc(sizeof(struct node_t));
  new_node->value = value;
  new_node->next = NULL;
  if (vec->head == NULL) {
    vec->head = new_node;
    vec->tail = new_node;
  } else {
    vec->tail->next = new_node;
    vec->tail = new_node;
  }
}

void vector_print(vector_t *vec) {
  struct node_t *current = vec->head;
  while (current != NULL) {
    printf("%d", current->value);
    current = current->next;
    if (current) {
      printf(",");
    }
  }
  printf("\n");
}

void register_excute_program(Register *reg, const int *program, int *index,
                             vector_t *output) {
  int op = program[*index];
  int literal = program[*index + 1];
  uint64_t combo = register_get_combo(reg, literal);
  switch (op) {
  case 0:
    reg->a = reg->a >> combo;
    *index += 2;
    break;
  case 1:
    reg->b = reg->b ^ literal;
    *index += 2;
    break;
  case 2:
    reg->b = combo % 8;
    *index += 2;
    break;
  case 3:
    if (reg->a == 0) {
      *index += 2;
    } else {
      *index = literal;
    }
    break;
  case 4:
    reg->b = reg->b ^ reg->c;
    *index += 2;
    break;
  case 5:
    vector_push(output, combo % 8);
    *index += 2;
    break;
  case 6:
    reg->b = reg->a >> combo;
    *index += 2;
    break;
  case 7:
    reg->c = reg->a >> combo;
    *index += 2;
    break;
  default:
    // This should never happen
    fprintf(stderr, "Invalid opcode: %d\n", op);
    free(output);
    free(reg);
    exit(EXIT_FAILURE);
  }
}

int find_a(const int *program, int program_size, uint64_t prev, int n,
           uint64_t *result) {
  if (n == 0) {
    *result = prev;
    return 1;
  }
  for (int i = 0; i < 8; ++i) {
    uint64_t a = prev << 3 | i;
    Register *reg = register_create(a, 0, 0);
    vector_t *output = vector_create();
    int index = 0;
    while (index < program_size - 2) {
      register_excute_program(reg, program, &index, output);
    }
    free(reg);
    int value = output->tail->value;
    vector_free(output);
    if (value != program[n - 1])
      continue;
    if (find_a(program, program_size, a, n - 1, result))
      return 1;
  }
  return 0;
}
