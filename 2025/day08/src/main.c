#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint32_t u32;
typedef size_t usize;

#define BUFSIZE 64

typedef struct {
  usize capacity, length, size;
  void *data;
} Vector;

Vector *vector_create(usize capacity, usize elem_size) {
  Vector *vec = malloc(sizeof(Vector));
  vec->capacity = capacity;
  vec->length = 0;
  vec->size = elem_size;
  vec->data = malloc(capacity * elem_size);
  return vec;
}

void vector_free(Vector *vec) {
  free(vec->data);
  free(vec);
}

void *vector_get(Vector *self, usize i) {
  if (i >= self->length)
    return NULL;
  return self->data + self->size * i;
}

void *vector_append(Vector *self) {
  if (self->length == self->capacity) {
    self->capacity *= 2;
    self->data = realloc(self->data, self->capacity * self->size);
  }
  return vector_get(self, self->length++);
}

typedef struct {
  u32 x, y, z;
} Point;

typedef struct {
  usize i, j;
  u32 dist;
} Pair;

void parse_line(const char *line, u32 *xyz) {
  usize n = strcspn(line, "\n");
  usize k = 0;
  u32 num = 0;
  for (usize i = 0; i <= n; ++i) {
    if (line[i] == ',' || i == n) {
      xyz[k++] = num;
      num = 0;
      continue;
    }
    num = num * 10 + line[i] - '0';
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(argv[1], "r");

  if (fp == NULL) {
    fprintf(stderr, "Could not open file '%s'\n", argv[1]);
    return EXIT_FAILURE;
  }
  char line[BUFSIZE];
  u32 xyz[3];

  Vector *points = vector_create(512, sizeof(Point));

  while (fgets(line, BUFSIZE, fp)) {
    parse_line(line, xyz);
    Point *point = (Point *)vector_append(points);
    point->x = xyz[0];
    point->y = xyz[1];
    point->z = xyz[2];
  }
  fclose(fp);

  vector_free(points);

  return EXIT_SUCCESS;
}
