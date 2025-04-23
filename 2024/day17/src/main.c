#include "chronospatial.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing intput file\n");
    return EXIT_FAILURE;
  }
  FILE *file = fopen(argv[1], "r");
  if (!file) {
    fprintf(stderr, "Could not open file '%s'\n", argv[1]);
    return EXIT_FAILURE;
  }
  uint64_t args[3] = {0, 0, 0};
  char buffer[BUFFER_SIZE];
  // Initalize register
  fgets(buffer, BUFFER_SIZE, file);
  for (int i = 0; i < 3; ++i) {
    int end = strcspn(buffer, "\n");
    int start = strcspn(buffer, ":");
    uint64_t value = 0;
    for (int j = start + 2; j < end; ++j) {
      value = value * 10 + (uint64_t)(buffer[j] - '0');
    }
    args[i] = value;
    fgets(buffer, BUFFER_SIZE, file);
  }
  Register *reg = register_create(args[0], args[1], args[2]);

  // Parse program
  int program[BUFFER_SIZE];
  fgets(buffer, BUFFER_SIZE, file);
  int start = strcspn(buffer, ":") + 2;
  int end = strcspn(buffer, "\n");
  int cntr = 0;
  for (int i = start; i < end; ++i) {
    if (buffer[i] == ',')
      continue;
    program[cntr++] = buffer[i] - '0';
  }
  fclose(file);

  vector_t *output = vector_create();
  int index = 0;
  while (index < cntr) {
    register_excute_program(reg, program, &index, output);
  }
  printf("Part 1: ");
  vector_print(output);
  free(reg);
  vector_free(output);

  uint64_t a = 0;
  if (find_a(program, cntr, 0, cntr, &a)) {
    printf("Part 2: %lu\n", a);
  }

  return EXIT_SUCCESS;
}
