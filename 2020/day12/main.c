#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#define BUFSIZE 64
#define ABS(a) (((a) < 0) ? -(a) : (a))

typedef int32_t i32;
typedef uint32_t u32;

enum Orientation { North, East, South, West };

typedef struct {
  i32 x, y;
  enum Orientation heading;
} ship_t;

typedef struct {
  char action;
  u32 value;
} instr_t;

void dir2vec(enum Orientation dir, i32 *dx, i32 *dy) {
  switch (dir) {
  case North:
    *dx = 0;
    *dy = 1;
    break;
  case East:
    *dx = 1;
    *dy = 0;
    break;
  case South:
    *dx = 0;
    *dy = -1;
    break;
  case West:
    *dx = -1;
    *dy = 0;
  }
}

void action2vec(ship_t *ship, char action, u32 value, i32 *dx, i32 *dy) {
  switch (action) {
  case 'F':
    dir2vec(ship->heading, dx, dy);
    break;
  case 'R':
    while (value > 0) {
      ship->heading = (ship->heading + 1) % 4;
      value -= 90;
    }
    break;
  case 'L':
    while (value > 0) {
      ship->heading = (ship->heading + 3) % 4;
      value -= 90;
    }
    break;
  case 'N':
    dir2vec(North, dx, dy);
    break;
  case 'E':
    dir2vec(East, dx, dy);
    break;
  case 'S':
    dir2vec(South, dx, dy);
    break;
  case 'W':
    dir2vec(West, dx, dy);
    break;
  default:
    assert(0);
    break;
  }
}

u32 solve_part1(vector *vec) {
  instr_t *instructions = (instr_t *)vec->data;
  ship_t ship = (ship_t){.x = 0, .y = 0, .heading = East};
  i32 dx, dy;
  for (u32 i = 0; i < vec->size; ++i) {
    instr_t cmd = instructions[i];
    action2vec(&ship, cmd.action, cmd.value, &dx, &dy);
    if (cmd.action == 'R' || cmd.action == 'L')
      continue;
    ship.x += cmd.value * dx;
    ship.y += cmd.value * dy;
  }
  return (u32)(ABS(ship.x) + ABS(ship.y));
}

u32 solve_part2(vector *vec) {
  instr_t *instructions = (instr_t *)vec->data;
  i32 ship_x = 0;
  i32 ship_y = 0;
  i32 waypoint_x = 10;
  i32 waypoint_y = 1;
  i32 dx, dy;
  u32 value;
  for (u32 i = 0; i < vec->size; ++i) {
    instr_t cmd = instructions[i];
    switch (cmd.action) {
    case 'F':
      ship_x += cmd.value * waypoint_x;
      ship_y += cmd.value * waypoint_y;
      break;
    case 'R':
      value = cmd.value;
      while (value > 0) {
        i32 tmp = waypoint_x;
        waypoint_x = waypoint_y;
        waypoint_y = -tmp;
        value -= 90;
      }
      break;
    case 'L':
      value = cmd.value;
      while (value > 0) {
        i32 tmp = waypoint_x;
        waypoint_x = -waypoint_y;
        waypoint_y = tmp;
        value -= 90;
      }
      break;
    case 'N':
      waypoint_y += cmd.value;
      break;
    case 'S':
      waypoint_y -= cmd.value;
      break;
    case 'E':
      waypoint_x += cmd.value;
      break;
    case 'W':
      waypoint_x -= cmd.value;
      break;
    default:
      assert(0);
      break;
    }
  }
  return (u32)(ABS(ship_x) + ABS(ship_y));
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
  vector *vec = VEC_CREATE(instr_t);
  while (fgets(buffer, BUFSIZE, fp)) {
    u32 n = strcspn(buffer, "\n");
    buffer[n] = '\0';
    u32 value = atoi(buffer + 1);
    instr_t instruction = (instr_t){.action = buffer[0], .value = value};
    VEC_PUSH(vec, instr_t, instruction);
  }
  fclose(fp);

  printf("Part 1: %u\n", solve_part1(vec));
  printf("Part 2: %u\n", solve_part2(vec));

  vector_free(vec);

  return EXIT_SUCCESS;
}
