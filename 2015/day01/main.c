#include "stdio.h"
#include "stdlib.h"

#define STRING_IMPLEMENTATION
#include "string8.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);

  int floor = 0;
  int part2 = -1;

  for (u64 i = 0; i < file.size - 1; ++i) {
    if (file.str[i] == '(') {
      floor++;
    } else {
      if (floor == 0 && part2 < 0)
        part2 = i + 1;
      floor--;
    }
  }

  printf("Part 1: %d\n", floor);
  printf("Part 2: %d\n", part2);

  free(file.str);
  return EXIT_SUCCESS;
}
