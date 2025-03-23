#include "stdio.h"
#include "string.h"

#define BUFFER_SIZE 256
#define MAX_SIZE 100
#define MAX_LINES 300

void parseCards(char *line, int n, int *cards, int *count) {
  int value = 0;
  for (int i = 0; i < n; ++i) {
    if (line[i] >= '0' && line[i] <= '9') {
      value = value * 10 + (line[i] - '0');
    } else if (value > 0) {
      cards[(*count)++] = value;
      value = 0;
    }
  }
  if (value > 0) {
    cards[(*count)++] = value;
  }
}

int isWinning(int card, int *winning, int nWinning) {
  for (int i = 0; i < nWinning; ++i) {
    if (card == winning[i]) {
      return 1;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return -1;
  }

  char buffer[BUFFER_SIZE];
  FILE *file = fopen(argv[1], "r");
  int hand[MAX_SIZE];
  int winning[MAX_SIZE];
  int counter[MAX_LINES];
  int part1 = 0;
  int iLine = 0;
  memset(counter, 0, sizeof(counter));
  while (fgets(buffer, BUFFER_SIZE, file)) {
    counter[iLine] += 1;
    int n = strcspn(buffer, "\n");
    int start = strcspn(buffer, ":");
    int sep = strcspn(buffer, "|");
    int nHand = 0;
    parseCards(buffer + start + 1, sep - start, hand, &nHand);
    int nWinning = 0;
    parseCards(buffer + sep + 1, n - sep, winning, &nWinning);
    int nWin = 0;
    int nextIndex = iLine + 1;
    for (int i = 0; i < nHand; ++i) {
      if (isWinning(hand[i], winning, nWinning)) {
        counter[nextIndex++] += counter[iLine];
        nWin++;
      }
    }
    part1 += nWin > 0 ? 1 << (nWin - 1) : 0;
    iLine++;
  }
  fclose(file);

  printf("Part 1: %d\n", part1);
  int part2 = 0;
  for (int i = 0; i < MAX_LINES; ++i) {
    part2 += counter[i];
  }
  printf("Part 2: %d\n", part2);

  return 0;
}
