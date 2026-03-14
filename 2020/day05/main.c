#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 64

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef uint32_t u32;
typedef u32 b32;

u32 parse_seat(char *seat) {
  u32 lo = 0;
  u32 hi = 127;
  u32 row;
  for (u32 i = 0; i < 7; ++i) {
    row = (lo + hi) / 2;
    if (seat[i] == 'F') {
      hi = row;
    } else {
      row++;
      lo = row;
    }
  }
  u32 col;
  lo = 0;
  hi = 7;
  for (u32 i = 7; i < 10; ++i) {
    col = (lo + hi) / 2;
    if (seat[i] == 'L') {
      hi = col;
    } else {
      col++;
      lo = col;
    }
  }
  return row * 8 + col;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "Could not open file %s\n", argv[1]);
    return EXIT_FAILURE;
  }
  char buffer[BUFSIZE];
  u32 n_seat = 127 * 8 + 7;
  b32 *touched = (b32 *)malloc(n_seat * sizeof(b32));
  for (u32 i = 0; i < n_seat; ++i) {
    touched[i] = false;
  }
  u32 part1 = 0;
  while (fgets(buffer, BUFSIZE, fp)) {
    u32 seat_id = parse_seat(buffer);
    part1 = MAX(part1, seat_id);
    touched[seat_id] = true;
  }
  fclose(fp);

  u32 part2;
  for (u32 i = 1; i < n_seat - 1; ++i) {
    if (!touched[i] && touched[i - 1] && touched[i + 1]) {
      part2 = i;
      break;
    }
  }
  free(touched);

  printf("Part 1: %u\n", part1);
  printf("Part 2: %u\n", part2);

  return EXIT_SUCCESS;
}
