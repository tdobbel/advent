#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

struct Splitter {
  int x;
  int y;
  uint64_t counter;
  int n_children;
  int touched;
  struct Splitter *chidren[2];
};

struct Splitter *create_splitter(int x, int y) {
  struct Splitter *splitter =
      (struct Splitter *)malloc(sizeof(struct Splitter));
  splitter->x = x;
  splitter->y = y;
  splitter->n_children = 0;
  splitter->touched = 0;
  splitter->counter = 0;
  splitter->chidren[0] = NULL;
  splitter->chidren[1] = NULL;
  return splitter;
}

void find_splits(struct Splitter **splitters, int isrc, int n_splitter, int nx,
                 int left) {
  struct Splitter *source = splitters[isrc];
  source->touched = 1;
  int x = source->x;
  if (left) {
    if (x == 0)
      return;
    x -= 1;
  } else {
    if (x == nx - 1)
      return;
    x += 1;
  }
  for (int i = isrc + 1; i < n_splitter; ++i) {
    struct Splitter *splitter = splitters[i];
    if (splitter->x == x) {
      source->chidren[source->n_children++] = splitter;
      if (!splitter->touched) {
        find_splits(splitters, i, n_splitter, nx, 0);
        find_splits(splitters, i, n_splitter, nx, 1);
      }
      return;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "Error opening %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  char buffer[BUFFER_SIZE];
  int startX, startY;
  int nx;
  int ny = 0;
  int n_splitter = 0;
  int capacity = BUFFER_SIZE;
  struct Splitter **splitters =
      (struct Splitter **)malloc(BUFFER_SIZE * sizeof(struct Splitter *));
  while (fgets(buffer, BUFFER_SIZE, fp)) {
    nx = strcspn(buffer, "\n");
    for (int i = 0; i < nx; ++i) {
      if (buffer[i] == 'S') {
        startX = i;
        startY = ny;
      } else if (buffer[i] == '^') {
        if (n_splitter == capacity) {
          capacity *= 2;
          splitters = (struct Splitter **)realloc(
              splitters, capacity * sizeof(struct Splitter *));
        }
        splitters[n_splitter++] = create_splitter(i, ny);
      }
    }
    ny++;
  }
  fclose(fp);
  splitters = (struct Splitter **)realloc(
      splitters, n_splitter * sizeof(struct Splitter *));

  find_splits(splitters, 0, n_splitter, nx, 0);
  find_splits(splitters, 0, n_splitter, nx, 1);
  int part1 = 0;
  for (int i = 0; i < n_splitter; i++) {
    if (splitters[i]->touched)
      part1++;
  }

  printf("Part 1: %d\n", part1);

  for (int i = n_splitter - 1; i >= 0; --i) {
    struct Splitter *splitter = splitters[i];
    if (!splitter->touched)
      continue;
    for (int ic = 0; ic < 2; ic++) {
      struct Splitter *child = splitter->chidren[ic];
      if (child) {
        splitter->counter += child->counter;
      } else {
        splitter->counter++;
      }
    }
  }

  printf("Part 2: %lu\n", splitters[0]->counter);

  for (int i = 0; i < n_splitter; i++) {
    free(splitters[i]);
  }
  free(splitters);

  return EXIT_SUCCESS;
}
