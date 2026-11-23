#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"
#define HASHMAP_IMPLEMENTATION
#include "hash_map.h"

#define WIDTH 7
#define N_SHAPE 5
#define X0 2
#define SPAWN_HEIGHT 3
#define NITER 2022

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef enum { Right, Down, Left } Direction;

typedef struct {
  u64 width, height;
  b8 *mask;
} Shape;

typedef struct {
  u64 x, y;
} Position;

typedef struct {
  Position ul;
  const Shape *shape;
} Rock;

Shape shapes[N_SHAPE] = {
    (Shape){.width = 4, .height = 1, .mask = (b8[4]){1, 1, 1, 1}},
    (Shape){
        .width = 3,
        .height = 3,
        .mask = (b8[9]){0, 1, 0, 1, 1, 1, 0, 1, 0},
    },
    (Shape){
        .width = 3, .height = 3, .mask = (b8[9]){0, 0, 1, 0, 0, 1, 1, 1, 1}},
    (Shape){.width = 1, .height = 4, .mask = (b8[4]){1, 1, 1, 1}},
    (Shape){.width = 2, .height = 2, .mask = (b8[4]){1, 1, 1, 1}}};

Rock rock_init(const Shape *shape, u64 top);
b8 rock_move(Rock *rock, Direction d, hash_map *used);
void rock_fall(Rock *rock, string8 jets, u64 *iptr, hash_map *used);
void debug_display(const Rock *rock, hash_map *used, u64 top);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);
  file = str_trim(file);

  hash_map *used = AUTO_HASHMAP(Position, b8);

  u64 top = 0;
  u64 iptr = 0;
  for (u64 iter = 0; iter < NITER; ++iter) {
    Rock rock = rock_init(&shapes[iter % N_SHAPE], top);
    rock_fall(&rock, file, &iptr, used);
    top = MAX(rock.ul.y, top);
  }

  printf("Part 1 = %lu\n", top);

  hm_deinit(used);
  free(file.str);

  return EXIT_SUCCESS;
}

Rock rock_init(const Shape *shape, u64 top) {
  Position ul = (Position){.x = X0, .y = top + shape->height + SPAWN_HEIGHT};
  return (Rock){.ul = ul, .shape = shape};
}

b8 rock_move(Rock *rock, Direction d, hash_map *used) {
  Position next_pos = rock->ul;
  switch (d) {
  case Left:
    if (next_pos.x == 0)
      return false;
    next_pos.x--;
    break;
  case Right:
    if (next_pos.x + rock->shape->width == WIDTH)
      return false;
    next_pos.x++;
    break;
  case Down:
    if (next_pos.y == rock->shape->height)
      return false;
    next_pos.y--;
    break;
  }
  for (u64 iy = 0; iy < rock->shape->height; ++iy) {
    for (u64 ix = 0; ix < rock->shape->width; ++ix) {
      if (!rock->shape->mask[iy * rock->shape->width + ix])
        continue;
      Position p = {next_pos.x + ix, next_pos.y - iy};
      if (hm_contains_key(used, &p))
        return false;
    }
  }
  rock->ul = next_pos;
  return true;
}

void rock_fall(Rock *rock, string8 jets, u64 *iptr, hash_map *used) {
  // debug_display(rock, used, rock->ul.y);
  while (true) {
    Direction d = jets.str[*iptr] == '>' ? Right : Left;
    rock_move(rock, d, used);
    *iptr = (*iptr + 1) % jets.size;
    if (!rock_move(rock, Down, used))
      break;
    // debug_display(rock, used, rock->ul.y);
  }
  // debug_display(rock, used, rock->ul.y);
  b8 dummy = true;
  for (u64 iy = 0; iy < rock->shape->height; ++iy) {
    for (u64 ix = 0; ix < rock->shape->width; ++ix) {
      if (!rock->shape->mask[iy * rock->shape->width + ix])
        continue;
      Position p = {rock->ul.x + ix, rock->ul.y - iy};
      hm_put(used, &p, &dummy);
    }
  }
}

void debug_display(const Rock *rock, hash_map *used, u64 top) {
  u64 xo = rock->ul.x;
  u64 yo = rock->ul.y;
  for (u64 y = top; y > 0; --y) {
    for (u64 x = 0; x < WIDTH; ++x) {
      b8 is_rock = false;
      if (x >= xo && x < xo + rock->shape->width && y <= yo &&
          y > yo - rock->shape->height) {
        u64 ix = x - xo;
        u64 iy = yo - y;
        is_rock = rock->shape->mask[iy * rock->shape->width + ix];
      }
      Position p = (Position){.x = x, .y = y};
      if (hm_contains_key(used, &p)) {
        if (is_rock) {
          printf("Couille dans le potage!\n");
        }
        printf("#");
      } else if (is_rock) {
        printf("@");
      } else {
        printf(".");
      }
    }
    printf("\n");
  }
  printf("-------\n\n");
}
