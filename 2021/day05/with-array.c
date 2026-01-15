#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define BUFSIZE 64

#define SIGN(a) (((a) == 0) ? 0 : (((a) > 0) ? 1 : -1))
#define ABS(a) (((a) >= 0) ? (a) : -(a))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

typedef uint32_t u32;
typedef uint16_t u16;
typedef size_t usize;
typedef int32_t i32;

struct Node {
  u16 a[2];
  u16 b[2];
  struct Node *next;
};

typedef struct {
  struct Node *head;
  struct Node *tail;
} LinkedList;

struct Node *parse_line(char *line) {
  struct Node *node = malloc(sizeof(struct Node));
  node->next = NULL;
  int end = strcspn(line, "\n");
  int stop = strcspn(line, " -> ");
  u16 v = 0;
  int i = 0, k = 0;
  for (i = 0; i <= stop; ++i) {
    if (line[i] == ',' || i == stop) {
      node->a[k++] = v;
      v = 0;
      continue;
    }
    v = v * 10 + line[i] - '0';
  }
  k = 0;
  for (i = stop + 4; i <= end; ++i) {
    if (line[i] == ',' || i == end) {
      node->b[k++] = v;
      v = 0;
      continue;
    }
    v = v * 10 + line[i] - '0';
  }
  return node;
}

LinkedList *vector_create() {
  LinkedList *ll = malloc(sizeof(LinkedList));
  ll->head = NULL;
  ll->tail = NULL;
  return ll;
}

void vector_append(LinkedList *ll, struct Node *node) {
  if (ll->head == NULL) {
    ll->head = node;
    ll->tail = node;
    return;
  }
  ll->tail->next = node;
  ll->tail = node;
}

void vector_free(LinkedList *ll) {
  struct Node *current = ll->head;
  while (current) {
    struct Node *next = current->next;
    free(current);
    current = next;
  }
  free(ll);
}

typedef struct {
  u16 *data;
  u16 xmin, ymin;
  usize nx, ny;
} Counter;

Counter *counter_create(u16 xmin, u16 xmax, u16 ymin, u16 ymax) {
  Counter *cntr = malloc(sizeof(Counter));
  cntr->nx = (usize)(xmax - xmin + 1);
  cntr->ny = (usize)(ymax - ymin + 1);
  cntr->xmin = xmin;
  cntr->ymin = ymin;
  usize size = cntr->nx * cntr->ny;
  cntr->data = malloc(sizeof(u16) * size);
  memset(cntr->data, 0, size * sizeof(u16));
  return cntr;
}

void counter_increment(Counter *cntr, u16 x, u16 y) {
  usize row = (usize)(y - cntr->ymin);
  usize col = (usize)(x - cntr->xmin);
  cntr->data[row * cntr->nx + col]++;
}

void counter_free(Counter *cntr) {
  free(cntr->data);
  free(cntr);
}

void path_increment(Counter *cntr, struct Node *node) {
  i32 dx = (i32)node->b[0] - (i32)node->a[0];
  i32 dy = (i32)node->b[1] - (i32)node->a[1];
  usize n = MAX(ABS(dx), ABS(dy));
  dx = SIGN(dx);
  dy = SIGN(dy);
  u16 x = node->a[0], y = node->a[1];
  for (usize i = 0; i <= n; ++i) {
    counter_increment(cntr, x, y);
    x += dx;
    y += dy;
  }
}

u32 count_overlaps(Counter *cntr) {
  u32 sum = 0;
  usize size = cntr->nx * cntr->ny;
  for (usize i = 0; i < size; ++i) {
    if (cntr->data[i] > 1)
      sum++;
  }
  return sum;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(argv[1], "r");
  if (!fp) {
    fprintf(stderr, "Could not open file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  char line[BUFSIZE];
  LinkedList *vec = vector_create();
  u16 xmin = UINT16_MAX, xmax = 0, ymin = UINT16_MAX, ymax = 0;
  while (fgets(line, BUFSIZE, fp)) {
    struct Node *node = parse_line(line);
    xmin = MIN(xmin, MIN(node->a[0], node->b[0]));
    xmax = MAX(xmax, MAX(node->a[0], node->b[0]));
    ymin = MIN(ymin, MIN(node->a[1], node->b[1]));
    ymax = MAX(ymax, MAX(node->a[1], node->b[1]));
    vector_append(vec, node);
  }
  fclose(fp);


  Counter *cntr1 = counter_create(xmin, xmax, ymin, ymax);
  Counter *cntr2 = counter_create(xmin, xmax, ymin, ymax);
  struct Node *node = vec->head;
  while (node) {
        if (node->a[0] == node->b[0] || node->a[1] == node->b[1]) {
      path_increment(cntr1, node);
    }
    path_increment(cntr2, node);
    node = node->next;
  }

  vector_free(vec);

  u32 part1 = count_overlaps(cntr1);
  printf("Part 1: %u\n", part1);
  counter_free(cntr1);

  u32 part2 = count_overlaps(cntr2);
  printf("Part 2: %u\n", part2);
  counter_free(cntr2);

  return EXIT_SUCCESS;
}
