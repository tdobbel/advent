#include "lan.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  Network *network = readNetwork(argv[1]);
  printf("Number of triangles: %d\n", countTriangles(network));
  char clique[256];
  findMaxClique(network, clique);
  printf("Largest clique: %s\n", clique);

  freeNetwork(network);

  return EXIT_SUCCESS;
}
