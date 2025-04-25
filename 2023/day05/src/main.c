#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256

struct Node {
  uint64_t a, b;
  struct Node *next;
};

typedef struct {
  struct Node *head;
  struct Node *tail;
} Vector;

typedef struct {
  uint64_t dst, src, rng;
} MapItem;

typedef struct {
  int size, capacity;
  MapItem *items;
} Map;

Vector *vecInit() {
  Vector *vec = malloc(sizeof(Vector));
  vec->head = NULL;
  vec->tail = NULL;
  return vec;
}

void vecPush(Vector *vec, uint64_t a, uint64_t b) {
  struct Node *node = malloc(sizeof(struct Node));
  node->a = a;
  node->b = b;
  node->next = NULL;
  if (!vec->head) {
    vec->head = node;
    vec->tail = node;
  } else {
    vec->tail->next = node;
    vec->tail = node;
  }
}

uint64_t vecMin(Vector *vec) {
  uint64_t minA = UINT64_MAX;
  struct Node *node = vec->head;
  while (node) {
    if (node->a < minA) {
      minA = node->a;
    }
    node = node->next;
  }
  return minA;
}

void vecFree(Vector *vec) {
  struct Node *current = vec->head;
  while (current) {
    struct Node *next = current->next;
    free(current);
    current = next;
  }
  free(vec);
}

void vecExtend(Vector *self, Vector *other) {
  struct Node *node = other->head;
  while (node) {
    vecPush(self, node->a, node->b);
    node = node->next;
  }
  vecFree(other);
}

int compareItems(const void *a, const void *b) {
  MapItem *ia = (MapItem *)a;
  MapItem *ib = (MapItem *)b;
  if (ia->src > ib->src)
    return 1;
  if (ia->src < ib->src)
    return -1;
  return 0;
}

void mapInit(Map *map) {
  map->size = 0;
  map->capacity = 24;
  map->items = malloc(sizeof(MapItem) * map->capacity);
}

void addItem(Map *map, uint64_t dst, uint64_t src, uint64_t rng) {
  if (map->size == map->capacity) {
    map->capacity *= 2;
    map->items = realloc(map->items, sizeof(MapItem) * map->capacity);
  }
  MapItem *item = &map->items[map->size++];
  item->dst = dst;
  item->src = src;
  item->rng = rng;
}

void mapSort(Map *map) {
  qsort(map->items, map->size, sizeof(MapItem), compareItems);
}

uint64_t applyMap(Map *map, uint64_t seed) {
  for (int i = 0; i < map->size; ++i) {
    MapItem *item = &map->items[i];
    if (seed >= item->src && seed < item->src + item->rng) {
      uint64_t offset = seed - item->src;
      return item->dst + offset;
    }
  }
  return seed;
}

Vector *applyMapToInterval(Map *map, uint64_t a, uint64_t b) {
  Vector *intervals = vecInit();
  uint64_t srcA = a;
  for (int i = 0; i < map->size; ++i) {
    MapItem *item = &map->items[i];
    if (item->src > b)
      break;
    if (srcA >= item->src + item->rng)
      continue;
    if (srcA < item->src) {
      vecPush(intervals, srcA, item->src);
      srcA = item->src;
    }
    uint64_t srcB = item->src + item->rng;
    if (b < srcB) {
      srcB = b;
    }
    uint64_t dstA = item->dst + srcA - item->src;
    uint64_t dstB = item->dst + srcB - item->src;
    vecPush(intervals, dstA, dstB);
    srcA = srcB;
    if (srcA >= b)
      break;
  }
  if (srcA < b) {
    vecPush(intervals, srcA, b);
  }
  return intervals;
}

uint64_t applyMaps(Map *maps, int nmap, uint64_t seed) {
  uint64_t result = seed;
  for (int i = 0; i < nmap; ++i) {
    result = applyMap(&maps[i], result);
  }
  return result;
}

Vector *applyMapsTointerval(Map *maps, int nmap, uint64_t src, uint64_t rng) {
  Vector *intervals = vecInit();
  vecPush(intervals, src, src + rng);
  for (int i = 0; i < nmap; ++i) {
    Vector *next = vecInit();
    struct Node *node = intervals->head;
    while (node) {
      Vector *results = applyMapToInterval(&maps[i], node->a, node->b);
      vecExtend(next, results);
      node = node->next;
    }
    vecFree(intervals);
    intervals = next;
  }
  return intervals;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  FILE *file = fopen(argv[1], "r");
  if (!file) {
    perror("Failed to open file");
    return EXIT_FAILURE;
  }
  char buffer[BUFFER_SIZE];
  int first = 1;
  uint64_t seeds[30];
  int nseed = 0;
  Map *maps = malloc(sizeof(Map) * 30);
  int nmap = 0;
  Map *current;
  uint64_t src, dst, rng;
  while (fgets(buffer, BUFFER_SIZE, file)) {
    int n = strcspn(buffer, "\n");
    buffer[n] = '\0';
    if (first) {
      int start = strcspn(buffer, ":") + 2;
      uint64_t value = 0;
      for (int i = start; i < n + 1; i++) {
        if (buffer[i] == ' ' || i == n) {
          seeds[nseed++] = value;
          value = 0;
        } else {
          value = 10 * value + (uint64_t)(buffer[i] - '0');
        }
      }
      first = 0;
      continue;
    }
    if (n == 0) {
      continue;
    }
    int icolon = strcspn(buffer, ":");
    if (icolon == n - 1) {
      current = &maps[nmap++];
      mapInit(current);
    } else {
      sscanf(buffer, "%lu %lu %lu", &dst, &src, &rng);
      addItem(current, dst, src, rng);
    }
  }
  fclose(file);

  maps = realloc(maps, sizeof(Map) * nmap);
  for (int i = 0; i < nmap; ++i) {
    mapSort(&maps[i]);
  }

  // Part 1
  uint64_t part1 = UINT64_MAX;
  for (int i = 0; i < nseed; ++i) {
    uint64_t val = applyMaps(maps, nmap, seeds[i]);
    if (val < part1) {
      part1 = val;
    }
  }
  printf("Part 1: %lu\n", part1);

  // Part 2
  uint64_t part2 = UINT64_MAX;
  for (int i = 0; i < nseed; i += 2) {
    Vector *vec = applyMapsTointerval(maps, nmap, seeds[i], seeds[i + 1]);
    uint64_t vmin = vecMin(vec);
    if (vmin < part2) {
      part2 = vmin;
    }
    vecFree(vec);
  }
  printf("Part 2: %lu\n", part2);

  // Clean everything up
  for (int i = 0; i < nmap; ++i) {
    Map *map = &maps[i];
    free(map->items);
  }
  free(maps);

  return EXIT_SUCCESS;
}
