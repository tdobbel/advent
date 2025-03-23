#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define BUFFERS_SIZE 256
#define MAX_SIZE 2048

typedef struct {
  int value, row, startcol, endcol;
} Number;

int isAdjacent(int symbol, Number *number, int nCols) {
  int x = symbol % nCols;
  int y = symbol / nCols;
  return (y >= number->row - 1 && y <= number->row + 1 &&
          x >= number->startcol - 1 && x <= number->endcol + 1);
}

int solvePart1(Number *numbers, int nNumber, int *symbols, int nSymbol,
               int nCols) {
  int sum = 0;
  for (int i = 0; i < nNumber; ++i) {
    Number *number = &numbers[i];
    for (int j = 0; j < nSymbol; ++j) {
      if (isAdjacent(symbols[j], number, nCols)) {
        sum += number->value;
        break;
      }
    }
  }
  return sum;
}

int solvePart2(Number *numbers, int nNumber, int *stars, int nStar, int nCols) {
  int sum = 0;
  for (int i = 0; i < nStar; ++i) {
    int nAdj = 0;
    int value = 1;
    for (int j = 0; j < nNumber && nAdj <= 2; ++j) {
      Number *number = &numbers[j];
      if (isAdjacent(stars[i], number, nCols)) {
        value *= number->value;
        nAdj++;
      }
    }
    if (nAdj == 2) {
      sum += value;
    }
  }
  return sum;
}

void addNumber(Number *numbers, int *nNumber, int *value, int row, int startcol,
               int endcol) {
  Number *number = &numbers[*nNumber];
  number->value = *value;
  number->row = row;
  number->startcol = startcol;
  number->endcol = endcol;
  (*nNumber)++;
  *value = 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return -1;
  }

  Number *numbers = (Number *)malloc(MAX_SIZE * sizeof(Number));
  int *symbols = (int *)malloc(MAX_SIZE * sizeof(int));
  int *stars = (int *)malloc(MAX_SIZE * sizeof(int));

  int nSymbol = 0;
  int nNumber = 0;
  int nStar = 0;
  int startcol, endcol;
  int nrows = 0;
  int ncols;
  FILE *file = fopen(argv[1], "r");
  char buffer[BUFFERS_SIZE];
  while (fgets(buffer, BUFFERS_SIZE, file)) {
    int value = 0;
    ncols = strcspn(buffer, "\n");
    for (int i = 0; i < ncols; ++i) {
      if (buffer[i] >= '0' && buffer[i] <= '9') {
        if (value == 0) {
          startcol = i;
        }
        value = value * 10 + buffer[i] - '0';
        endcol = i;
        continue;
      }
      if (value > 0) {
        addNumber(numbers, &nNumber, &value, nrows, startcol, endcol);
      }
      if (buffer[i] == '.') {
        continue;
      }
      symbols[nSymbol++] = nrows * ncols + i;
      if (buffer[i] == '*') {
        stars[nStar++] = nrows * ncols + i;
      }
    }
    if (value > 0) {
      addNumber(numbers, &nNumber, &value, nrows, startcol, endcol);
    }
    nrows++;
  }

  fclose(file);

  int part1 = solvePart1(numbers, nNumber, symbols, nSymbol, ncols);
  printf("Part 1: %d\n", part1);
  int part2 = solvePart2(numbers, nNumber, stars, nStar, ncols);
  printf("Part 2: %d\n", part2);

  free(numbers);
  free(symbols);
  free(stars);

  return 0;
}
