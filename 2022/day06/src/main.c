#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t MAX_LINE_LENGTH = 4096;

int contains(char *array, size_t n, char c) {
  for (size_t i = 0; i < n; ++i) {
    if (array[i] == c) {
      return 1;
    }
  }
  return 0;
}

int solve(const char *line, size_t line_size, size_t word_size) {
  char word[word_size];
  for (size_t start = 0; start < line_size - word_size; ++start) {
    word[0] = line[start];
    int ok = 1;
    for (size_t i = 1; i < word_size; ++i) {
      if (contains(word, i, line[start + i])) {
        ok = 0;
        break;
      } else {
        word[i] = line[start + i];
      }
    }
    if (ok == 1) {
      return start + word_size;
    }
  }
  return -1;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }
  FILE *file = fopen(argv[1], "r");
  char line[MAX_LINE_LENGTH];
  fgets(line, MAX_LINE_LENGTH, file);
  fclose(file);
  size_t line_size = strcspn(line, "\n");
  printf("Part 1: %d\n", solve(line, line_size, 4));
  printf("Part 2: %d\n", solve(line, line_size, 14));
  return 0;
}
