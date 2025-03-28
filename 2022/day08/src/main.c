#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256

int isVisible(int pos, int *trees, int nrows, int ncols) {
  int row = pos / ncols;
  int col = pos % ncols;
  int visibleTop = 1, visibleBottom = 1, visibleLeft = 1, visibleRight = 1;
  for (int i = 0; i < row && visibleTop; ++i) {
    visibleTop = trees[i * ncols + col] < trees[pos];
  }
  if (visibleTop)
    return 1;
  for (int i = nrows - 1; i > row && visibleBottom; --i) {
    visibleBottom = trees[i * ncols + col] < trees[pos];
  }
  if (visibleBottom)
    return 1;
  for (int i = 0; i < col && visibleLeft; ++i) {
    visibleLeft = trees[row * ncols + i] < trees[pos];
  }
  if (visibleLeft)
    return 1;
  for (int i = ncols - 1; i > col && visibleRight; --i) {
    visibleRight = trees[row * ncols + i] < trees[pos];
  }
  return visibleRight;
}

int scenicScore(int pos, int *trees, int nrows, int ncols) {
  int row = pos / ncols;
  int col = pos % ncols;
  int scoreTop = 0, scoreBottom = 0, scoreLeft = 0, scoreRight = 0;
  int i;
  for (i = row - 1; i >= 0; --i) {
    scoreTop++;
    if (trees[i * ncols + col] >= trees[pos])
      break;
  }
  for (i = row + 1; i < nrows; ++i) {
    scoreBottom++;
    if (trees[i * ncols + col] >= trees[pos])
      break;
  }
  for (i = col - 1; i >= 0; --i) {
    scoreLeft++;
    if (trees[row * ncols + i] >= trees[pos])
      break;
  }
  for (i = col + 1; i < ncols; ++i) {
    scoreRight++;
    if (trees[row * ncols + i] >= trees[pos])
      break;
  }
  return scoreTop * scoreBottom * scoreLeft * scoreRight;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  if (!file) {
    printf("Error opening file\n");
    return 1;
  }
  int ncols, i, nrows = 0;
  char buffer[BUFFER_SIZE];
  int *trees;
  while (fgets(buffer, BUFFER_SIZE, file)) {
    ncols = strcspn(buffer, "\n");
    if (nrows == 0) {
      trees = (int *)malloc(ncols * 200 * sizeof(int));
    }
    for (i = 0; i < ncols; ++i) {
      trees[nrows * ncols + i] = buffer[i] - '0';
    }
    nrows++;
  }
  fclose(file);

  trees = (int *)realloc(trees, nrows * ncols * sizeof(int));
  int part1 = 0;
  int part2 = 0;
  int score;
  for (i = 0; i < nrows * ncols; ++i) {
    part1 += isVisible(i, trees, nrows, ncols);
    score = scenicScore(i, trees, nrows, ncols);
    if (score > part2) {
      part2 = score;
    }
  }
  printf("Part 1: %d\n", part1);
  printf("Part 2: %d\n", part2);

  free(trees);
  return 0;
}
