#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint32_t u32;
typedef size_t usize;

#define BUFSIZE 64
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

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
  u32 distance;
} Pair;

struct Node {
  usize pid;
  struct Node *next;
};

typedef struct {
  usize length;
  struct Node *head;
  struct Node *tail;
} LinkedList;

typedef struct {
  usize size;
  LinkedList *parts;
} Circuits;

Circuits *circuits_create(usize size) {
  Circuits *circuits = (Circuits *)malloc(sizeof(Circuits));
  circuits->size = size;
  circuits->parts = (LinkedList *)malloc(sizeof(LinkedList) * size);
  for (usize i = 0; i < size; ++i) {
    struct Node *node = malloc(sizeof(struct Node));
    node->pid = i;
    node->next = NULL;
    LinkedList *ll = circuits->parts + i;
    ll->head = node;
    ll->tail = node;
    ll->length = 1;
  }
  return circuits;
}

void circuits_free(Circuits *circuits) {
  for (usize i = 0; i < circuits->size; ++i) {
    LinkedList *ll = &circuits->parts[i];
    struct Node *node = ll->head;
    while (node) {
      struct Node *next = node->next;
      free(node);
      node = next;
    }
  }
  free(circuits->parts);
  free(circuits);
}

void circuits_connect(Circuits *circuits, Pair *pair) {
  usize k = 0;
  usize parts[2];
  for (usize i = 0; i < circuits->size; ++i) {
    LinkedList *ll = &circuits->parts[i];
    struct Node *node = ll->head;
    while (node) {
      if (node->pid == pair->i || node->pid == pair->j) {
        parts[k++] = i;
      }
      node = node->next;
    }
    if (k == 2)
      break;
  }
  if (parts[0] == parts[1]) {
    // Already connected, nothing to be done
    return;
  }
  LinkedList *src = &circuits->parts[parts[1]];
  LinkedList *dst = &circuits->parts[parts[0]];
  dst->tail->next = src->head;
  dst->tail = src->tail;
  dst->length += src->length;
  src->length = 0;
  src->head = NULL;
  src->tail = NULL;
  usize ilast = --circuits->size;
  circuits->parts[parts[1]] = circuits->parts[ilast];
  circuits->parts[ilast] = *src;
}

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

u32 compute_distance(const Point *p0, const Point *p1) {
  u32 dx = p1->x - p0->x;
  u32 dy = p1->y - p0->y;
  u32 dz = p1->z - p0->z;
  return dx * dx + dy * dy + dz * dz;
}

void compute_pairwise_distance(Vector *points, Vector *pairs, u32 max_dist,
                               usize iref) {
  const Point *p0 = vector_get(points, iref);
  for (usize i = iref + 1; i < points->length; ++i) {
    const Point *p1 = vector_get(points, i);
    u32 dist = compute_distance(p0, p1);
    if (dist > max_dist)
      continue;
    Pair *pair = (Pair *)vector_append(pairs);
    pair->i = iref;
    pair->j = i;
    pair->distance = dist;
  }
}

int compare_pairs(const void *a, const void *b) {
  Pair *pairA = (Pair *)a;
  Pair *pairB = (Pair *)b;
  if (pairA->distance < pairB->distance)
    return -1;
  if (pairA->distance > pairB->distance)
    return 1;
  return 0;
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
  u32 xmin = UINT32_MAX, ymin = UINT32_MAX, zmin = UINT32_MAX;
  u32 xmax = 0, ymax = 0, zmax = 0;

  while (fgets(line, BUFSIZE, fp)) {
    parse_line(line, xyz);
    Point *point = (Point *)vector_append(points);
    point->x = xyz[0];
    point->y = xyz[1];
    point->z = xyz[2];
    xmin = MIN(xmin, point->x);
    ymin = MIN(ymin, point->y);
    zmin = MIN(zmin, point->z);
    xmax = MAX(xmax, point->x);
    ymax = MAX(ymax, point->y);
    zmax = MAX(zmax, point->z);
  }
  fclose(fp);

  Point pmin = {.x = xmin, .y = ymin, .z = zmin};
  Point pmax = {.x = xmax, .y = ymax, .z = zmax};
  u32 max_dist = compute_distance(&pmin, &pmax);

  Vector *pairs = vector_create(512, sizeof(Pair));
  for (usize i = 0; i < points->size - 1; i++) {
    compute_pairwise_distance(points, pairs, max_dist, i);
  }
  qsort(pairs->data, pairs->length, sizeof(Pair), compare_pairs);

  Circuits *circuits = circuits_create(points->length);
  usize ipair = 0;
  for (usize iloop = 0; iloop < 10; iloop++) {
    Pair *pair = (Pair *)vector_get(pairs, ipair++);
    Point *pa = (Point *)vector_get(points, pair->i);
    Point *pb = (Point *)vector_get(points, pair->j);
    printf("Connection (%u, %u, %u) with (%u, %u, %u)\n", pa->x, pa->y, pa->z,
           pb->x, pb->y, pb->z);
    circuits_connect(circuits, pair);
    for (usize ic = 0; ic < circuits->size; ++ic) {
      printf("Circuit %2lu : %lu\n", ic, circuits->parts[ic].length);
    }
  }

  vector_free(points);
  vector_free(pairs);
  circuits_free(circuits);

  return EXIT_SUCCESS;
}
