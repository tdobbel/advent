#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define BASE_CAPACITY 1024

typedef uint64_t u64;

typedef struct {
  u64 elem_size, capacity, size;
  void *data;
} vector;

vector *vector_create(u64 elem_size);
void vector_free(vector *vec);
void *vector_get(vector *vec, u64 index);
void *vector_append_get(vector *vec);

#define VEC_CREATE(T) vector_create(sizeof(T))
#define VEC_PUSH(vec, T, x) (*(T *)vector_append_get((vec)) = (x))

#ifdef VECTOR_IMPLEMENTATION

vector *vector_create(u64 elem_size) {
  vector *vec = (vector *)malloc(sizeof(vector));
  vec->size = 0;
  vec->elem_size = elem_size;
  vec->capacity = BASE_CAPACITY;
  vec->data = malloc(BASE_CAPACITY * elem_size);
  return vec;
}

void *vector_get(vector *vec, u64 index) {
  assert(vec && index < vec->size);
  return (void *)((char *)vec->data + index * vec->elem_size);
}

void *vector_append_get(vector *vec) {
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, vec->capacity * vec->elem_size);
  }
  return vector_get(vec, vec->size++);
}

void vector_free(vector *vec) {
  if (!vec)
    return;
  free(vec->data);
  free(vec);
}

#endif

#endif
