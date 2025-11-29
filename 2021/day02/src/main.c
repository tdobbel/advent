#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 64

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return EXIT_FAILURE;
  }
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    printf("Error when opening file\n");
    return EXIT_FAILURE;
  }
  char buffer[BUFFER_SIZE];
  char direction[7];
  int x = 0, y = 0, aim = 0;
  while (fgets(buffer, BUFFER_SIZE, fp)) {
    int flag = 0;
    int n = 0;
    for (int i = 0; i < BUFFER_SIZE; ++i) {
      if (buffer[i] == '\n')
        break;
      if (flag) {
        n = 10 * n + buffer[i] - '0';
        continue;
      }
      if (buffer[i] == ' ') {
        flag = 1;
        direction[i] = '\0';
        continue;
      }
      direction[i] = buffer[i];
    }
    if (strncmp(direction, "forward", 7) == 0) {
      x += n;
      y += aim * n;
    } else if (strncmp(direction, "down", 4) == 0) {
      aim += n;
    } else if (strncmp(direction, "up", 2) == 0) {
      aim -= n;
    } else {
      printf("Unknown diretion '%s'\n", direction);
    }
  }
  fclose(fp);

  printf("Part1: %d\n", x * aim);
  printf("Part2: %d\n", x * y);

  return EXIT_SUCCESS;
}
