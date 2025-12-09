#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

struct Splitter {
  int x;
  int y;
  int n_children;
  struct Splitter *chidren[2];
};

struct Splitter *create_splitter(int x, int y) {
  struct Splitter *splitter =
      (struct Splitter *)malloc(sizeof(struct Splitter));
  splitter->x = x;
  splitter->y = y;
  splitter->n_children = 0;
  return splitter;
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
  int capacity = BUFFER_SIZE;
  int size = 0;
  char *content = (char *)malloc(capacity);
  int n_splitter = 0;
  struct Splitter **splitters =
      (struct Splitter **)malloc(BUFFER_SIZE * sizeof(struct Splitter *));
  while (fgets(buffer, BUFFER_SIZE, fp)) {
    nx = strcspn(buffer, "\n");
    for (int i = 0; i < nx; ++i) {
      if (buffer[i] == 'S') {
        startX = i;
        startY = ny;
      } else if (buffer[i] == '^') {
        splitters[n_splitter++] = create_splitter(i, ny);
      }
      if (size >= capacity) {
        capacity *= 2;
        content = (char *)realloc(content, capacity);
      }
      content[size++] = buffer[i];
    }
    ny++;
  }
  fclose(fp);
  content = (char *)realloc(content, size);
  splitters = (struct Splitter **)realloc(
      splitters, n_splitter * sizeof(struct Splitter *));

  // printf("%d %d %d", nx, ny, size);
  //
  char **manifold = (char **)malloc(ny * sizeof(char *));
  manifold[0] = content;
  for (int i = 1; i < ny; ++i) {
    manifold[i] = manifold[i - 1] + nx;
  }

  free(content);
  free(manifold);
  for (int i = 0; i < n_splitter; i++) {
    printf("%d, %d\n", splitters[i]->x, splitters[i]->y);
    free(splitters[i]);
  }
  free(splitters);

  return EXIT_SUCCESS;
}
