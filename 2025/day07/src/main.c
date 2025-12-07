#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

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
  int nx;
  int ny = 0;
  int capacity = BUFFER_SIZE;
  int size = 0;
  int startX, startY;
  char *content = (char *)malloc(capacity);
  while (fgets(buffer, BUFFER_SIZE, fp)) {
    nx = strcspn(buffer, "\n");
    for (int i = 0; i < nx; ++i) {
      if (buffer[i] == 'S') {
        startX = i;
        startY = ny;
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

  // printf("%d %d %d", nx, ny, size);
  //
  char **manifold = (char **)malloc(ny * sizeof(char *));
  manifold[0] = content;
  for (int i = 1; i < ny; ++i) {
    manifold[i] = manifold[i - 1] + nx;
  }

  free(content);
  free(manifold);

  return EXIT_SUCCESS;
}
