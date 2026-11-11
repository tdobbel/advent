#include <stdio.h>
#include <stdlib.h>

#define ARENA_IMPLEMENTATION

#include "arena.h"

#define STRING_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION
#define HASHMAP_IMPLEMENTATION

#include "hash_map.h"
#include "string8.h"
#include "vector.h"

typedef enum { Up, Right, Down, Left } Direction;

typedef struct {
  Direction dir;
  u32 n_step;
} Motion;

typedef struct {
  i32 x, y;
} Position;

typedef struct {
  u32 size;
  Position *knots;
} Rope;

#define ABS(a) ((a) >= 0 ? (a) : -(a))

void rope_init(mem_arena *gpa, Rope *rope, u64 size);
void rope_move(Rope *rope, Motion motion, hash_map *visited);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);
  vector *line_vec = VEC_CREATE(string8);
  str_split(line_vec, str_trim(file), STR8_LIT("\n"));

  mem_arena *gpa = arena_create(KiB(500));
  u32 nmov = line_vec->size;
  Motion *motions = ALLOC_ARRAY(gpa, Motion, nmov);

  string8 *lines = VEC_CAST(line_vec, string8);
  for (u32 i = 0; i < line_vec->size; ++i) {
    Direction dir;
    switch (lines[i].str[0]) {
    case 'U':
      dir = Up;
      break;
    case 'R':
      dir = Right;
      break;
    case 'D':
      dir = Down;
      break;
    case 'L':
      dir = Left;
      break;
    default:
      fprintf(stderr, "Invalid direction character '%c'\n", lines[i].str[0]);
      return EXIT_FAILURE;
    }
    u32 n_step = str_parse_unsigned(
        (string8){.str = lines[i].str + 2, .size = lines[i].size - 2});
    motions[i] = (Motion){.dir = dir, .n_step = n_step};
  }

  free(file.str);
  vector_deinit(line_vec);

  Rope rope = {0};
  hash_map *visited = AUTO_HASHMAP(Position, u8);
  rope_init(gpa, &rope, 2);
  for (u32 imov = 0; imov < nmov; ++imov) {
    rope_move(&rope, motions[imov], visited);
  }
  printf("Part 1: %lu\n", visited->size);

  hm_reset(visited);
  rope_init(gpa, &rope, 10);
  for (u32 imov = 0; imov < nmov; ++imov) {
    rope_move(&rope, motions[imov], visited);
  }
  printf("Part 2: %lu\n", visited->size);

  hm_deinit(visited);

  arena_destroy(gpa);

  return EXIT_SUCCESS;
}

void rope_init(mem_arena *gpa, Rope *rope, u64 size) {
  rope->size = size;
  rope->knots = ALLOC_ARRAY(gpa, Position, size);
  for (u32 i = 0; i < size; ++i) {
    rope->knots[i].x = 0;
    rope->knots[i].y = 0;
  }
}

void rope_move(Rope *rope, Motion motion, hash_map *visited) {
  u8 dummy = 1;
  i32 dx = 0, dy = 0;
  switch (motion.dir) {
  case Up:
    dy = 1;
    break;
  case Right:
    dx = 1;
    break;
  case Down:
    dy = -1;
    break;
  case Left:
    dx = -1;
    break;
  }
  for (u32 step = 0; step < motion.n_step; ++step) {
    rope->knots[0].x += dx;
    rope->knots[0].y += dy;
    Position pos = rope->knots[0];
    for (u32 i = 1; i < rope->size; ++i) {
      i32 dx2 = pos.x - rope->knots[i].x;
      i32 dy2 = pos.y - rope->knots[i].y;
      i32 abs_dx2 = ABS(dx2);
      i32 abs_dy2 = ABS(dy2);
      if (abs_dx2 > 1 || abs_dy2 > 1) {
        rope->knots[i].x += abs_dx2 > 1 ? dx2 / abs_dx2 : dx2;
        rope->knots[i].y += abs_dy2 > 1 ? dy2 / abs_dy2 : dy2;
      }
      pos = rope->knots[i];
    }
    hm_put(visited, &pos, &dummy);
  }
}
