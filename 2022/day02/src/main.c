#include <stdio.h>

#define BUFFER_SIZE 16

const char moves1[3] = {'A', 'B', 'C'};
const char moves2[3] = {'X', 'Y', 'Z'};

void parseMoves(char move1, char move2, int *value1, int *value2) {
  for (int k = 0; k < 3; ++k) {
    if (move1 == moves1[k])
      *value1 = k + 1;
    if (move2 == moves2[k])
      *value2 = k + 1;
  }
}

int computeScore(int value1, int value2) {
  if (value2 == value1 % 3 + 1)
    return value2 + 6;
  if (value1 == value2)
    return value2 + 3;
  return value2;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return 1;
  }
  FILE *file = fopen(argv[1], "r");
  char buffer[BUFFER_SIZE];
  char player1, player2;
  int value1, value2;
  int part1 = 0, part2 = 0;
  while (fgets(buffer, BUFFER_SIZE, file)) {
    sscanf(buffer, "%c %c", &player1, &player2);
    parseMoves(player1, player2, &value1, &value2);
    part1 += computeScore(value1, value2);
    if (value2 == 1) {
      value2 = (value1 + 1) % 3 + 1;
    } else if (value2 == 2) {
      value2 = value1;
    } else {
      value2 = value1 % 3 + 1;
    }
    part2 += computeScore(value1, value2);
  }
  printf("Part 1: %d\n", part1);
  printf("Part 2: %d\n", part2);
  fclose(file);
  return 0;
}
