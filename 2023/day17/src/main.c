#include "libmaze.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  int n_pixel, size;
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  city_t *city = city_from_file(argv[1]);
  int part1 = find_shortest_path(city, 0, 3);
  printf("Part 1: %d\n", part1);
  int part2 = find_shortest_path(city, 4, 10);
  printf("Part 2: %d\n", part2);

  city_free(city);

  return EXIT_SUCCESS;
}
