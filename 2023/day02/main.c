#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#define BUFFER_SIZE 256

const int CAPACITY[3] = {12, 13, 14};
const char *COLORS[3] = {"red", "green", "blue"};

int parseColor(char *color) {
  for (int k = 0; k < 3; ++k) {
    if (strcmp(color, COLORS[k]) == 0)
      return k;
  }
  perror("Color not recognized\n");
  return -1;
}

void parseCubes(char *buffer, int *counter) {
  char *line = strdup(buffer);
  char *token;
  int size, value, i;
  while ((token = strsep(&line, ",")) != NULL) {
    size = strlen(token);
    value = 0;
    for (i = 0; i < size; ++i) {
      if (token[i] == ' ')
        continue;
      if (token[i] >= '0' && token[i] <= '9') {
        value = 10 * value + (token[i] - '0');
        continue;
      }
      break;
    }
    counter[parseColor(token + i)] = value;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return -1;
  }

  FILE *file = fopen(argv[1], "r");
  char buffer[BUFFER_SIZE];
  char *found;
  int gameID = 1, i, valid;
  int part1 = 0;
  int part2 = 0;
  while (fgets(buffer, BUFFER_SIZE, file)) {
    buffer[strcspn(buffer, "\n")] = '\0';
    int start = strcspn(buffer, ":");
    char *line = strdup(buffer + start + 1);
    int required[3] = {0, 0, 0};
    valid = 1;
    while ((found = strsep(&line, ";")) != NULL) {
      int counter[3] = {0, 0, 0};
      parseCubes(found, counter);
      for (i = 0; i < 3; ++i) {
        if (counter[i] > CAPACITY[i])
          valid = 0;
        if (counter[i] > required[i])
          required[i] = counter[i];
      }
    }
    if (valid) {
      part1 += gameID;
    }
    part2 += required[0] * required[1] * required[2];
    gameID++;
  }
  fclose(file);

  printf("Part 1: %d\n", part1);
  printf("Part 2: %d\n", part2);

  return 0;
}
