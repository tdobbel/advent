#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256

int priority(char c) {
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 1;
  if (c >= 'A' && c <= 'Z') {
    return 27 + c - 'A';
  }
  return -1;
}

char findCommon(char *ref, char *candidates[], int nCandidates) {
  int common;
  int size = strlen(ref);
  for (int i = 0; i < size; ++i) {
    common = 1;
    char needle[1] = {ref[i]};
    for (int j = 0; j < nCandidates; ++j) {
      char *p = strstr(candidates[j], needle);
      if (!p) {
        common = 0;
        break;
      }
    }
    if (common) {
      return ref[i];
    }
  }
  return '\0';
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return 1;
  }

  FILE *file = fopen(argv[1], "r");

  char buffer[BUFFER_SIZE];
  char left[BUFFER_SIZE], right[BUFFER_SIZE];
  int size, mid;
  int part1 = 0, part2 = 0;
  char *group[3];
  int cntr = 0;
  while (fgets(buffer, BUFFER_SIZE, file)) {
    size = strcspn(buffer, "\n");
    buffer[size] = '\0';
    group[cntr % 3] = strdup(buffer);
    mid = size / 2;
    strncpy(left, buffer, mid);
    left[mid] = '\0';
    strncpy(right, buffer + mid, size - mid);
    right[size - mid] = '\0';
    part1 += priority(findCommon(left, (char *[]){right}, 1));
    cntr++;
    if (cntr % 3 == 0) {
      part2 += priority(findCommon(group[0], group + 1, 2));
      for (int k = 0; k < 3; ++k) {
        free(group[k]);
      }
    }
  }
  fclose(file);

  printf("Part 1: %d\n", part1);
  printf("Part 2: %d\n", part2);

  return 0;
}
