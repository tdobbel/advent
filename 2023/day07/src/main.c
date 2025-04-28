#include "camel_cards.h"
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 16

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  FILE *file = fopen(argv[1], "r");
  if (!file) {
    perror("Error opening file");
    return EXIT_FAILURE;
  }
  char buffer[BUFFER_SIZE];
  HandList *handList = createHandList(64);
  while (fgets(buffer, BUFFER_SIZE, file)) {
    Hand *hand = parseHand(buffer);
    addHand(handList, hand);
  }
  fclose(file);

  sortHandList(handList);
  int part1 = computeScore(handList);
  printf("Part 1: %d\n", part1);
  int part2 = computeJokerScore(handList);
  printf("Part 2: %d\n", part2);

  freeHandList(handList);

  return EXIT_SUCCESS;
}
