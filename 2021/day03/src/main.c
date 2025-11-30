#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 16

int filter(char **report, int start, int size, int num_size, int flag_oxygen) {
  for (int i = 0; i < num_size; ++i) {
    int j = start;
    while (j < start + size) {
      while (j < start + size && report[j][i] == '0') {
        j++;
      }
      int k = j;
      while (k < start + size && report[k][i] == '1') {
        k++;
      }
      if (k == start + size) {
        break;
      }
      char *tmp = report[j];
      report[j] = report[k];
      report[k] = tmp;
      j++;
    }
    int n_zeros = j - start;
    int n_ones = size - j + start;
    if (flag_oxygen ^ (n_zeros > n_ones)) {
      start = j;
      size = n_ones;
    } else {
      size = n_zeros;
    }
    if (size == 1)
      return start;
  }
  return -1;
}

int to_decimal(char *bits, int n) {
  int res = 0;
  for (int i = 0; i < n; i++) {
    res <<= 1;
    res += bits[i] - '0';
  }
  return res;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(argv[1], "r");
  char buffer[BUFFER_SIZE];
  int capacity = 128;
  char **report = malloc(capacity * sizeof(char *));
  int n_entry = 0;
  int num_size = 0;
  int *counter = NULL;
  while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
    // reallocate if needed
    if (n_entry == capacity) {
      capacity *= 2;
      report = realloc(report, capacity * sizeof(char *));
    }
    // Get number size and initialize counter
    if (counter == NULL) {
      while (buffer[num_size] != '\n') {
        num_size++;
      }
      counter = malloc(num_size * sizeof(int));
      for (int i = 0; i < num_size; ++i) {
        counter[i] = 0;
      }
    }
    buffer[num_size] = '\0';
    // Add to report
    for (int i = 0; i < num_size; ++i) {
      counter[i] += buffer[i] - '0';
    }
    report[n_entry] = malloc(BUFFER_SIZE);
    strncpy(report[n_entry], buffer, BUFFER_SIZE);
    n_entry++;
  }
  fclose(fp);

  report = realloc(report, n_entry * sizeof(char *));
  int gamma = 0;
  int epsilon = 0;
  for (int i = 0; i < num_size; ++i) {
    gamma <<= 1;
    epsilon <<= 1;
    if (counter[i] > n_entry / 2) {
      gamma++;
    } else {
      epsilon++;
    }
  }
  printf("Part 1: %d\n", gamma * epsilon);
  free(counter);

  int iox = filter(report, 0, n_entry, num_size, 1);
  int oxygenRating = to_decimal(report[iox], num_size);
  int ico = filter(report, 0, n_entry, num_size, 0);
  int co2Rating = to_decimal(report[ico], num_size);

  printf("Part 2: %d\n", oxygenRating * co2Rating);

  for (int i = 0; i < n_entry; ++i) {
    free(report[i]);
  }
  free(report);

  return EXIT_SUCCESS;
}
