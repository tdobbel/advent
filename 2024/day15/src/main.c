#include <stdio.h>
#include <stdlib.h>
#define STRING_IMPLEMENTATION
#include "string8.h"
#define VECTOR_IMPLEMENTATION
#include "vector.h"

#include "arena.h"

typedef struct {
  u32 xo, yo;
  u32 width;
} aoc_box;

typedef struct {
  u32 ny, nx, nobj;
  u32 robotX, robotY;
  aoc_box *boxes;
  i32 *grid;
  vector *to_check, *to_move;
} aoc_puzzle;

b8 eql(const void *a, const void *b) { return *(u32 *)a == *(u32 *)b; }
void parse_move(u8 move, i32 *dx, i32 *dy);
void puzzle_display(aoc_puzzle *puzzle);
void puzzle_move(aoc_puzzle *puzzle, u8 move);
u64 puzzle_gps_score(aoc_puzzle *puzzle);
u64 puzzle_solve(aoc_puzzle *puzzle, string8 instructions);
void puzzle_grow(aoc_puzzle *dst, mem_arena *arena, const aoc_puzzle *src);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf(("Missing input file\n"));
    return EXIT_FAILURE;
  }

  mem_arena *perm_arena = arena_create(KiB(500));

  string8 file = {0};
  str_read_file(perm_arena, &file, argv[1]);

  vector *line_vec = VEC_ARENA_CREATE(perm_arena, string8);
  str_split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;

  vector *grid = VEC_ARENA_CREATE(perm_arena, i32);
  vector *boxes = VEC_ARENA_CREATE(perm_arena, aoc_box);
  u32 ny = 0, nx = 0, robotX = 0, robotY = 0;
  u64 i;
  i32 iobj = 1;
  for (i = 0; i < line_vec->size && lines[i].size > 0; ++i) {
    nx = lines[i].size;
    for (u32 x = 0; x < nx; ++x) {
      switch (lines[i].str[x]) {
      case '#':
        VEC_PUSH(grid, i32, -1);
        break;
      case 'O':
        VEC_PUSH(grid, i32, iobj++);
        aoc_box box = (aoc_box){.xo = x, .yo = ny, .width = 1};
        VEC_PUSH(boxes, aoc_box, box);
        break;
      case '@':
        robotX = x;
        robotY = ny;
        VEC_PUSH(grid, i32, 0);
        break;
      default:
        VEC_PUSH(grid, i32, 0);
        break;
      }
    }
    ny++;
  }

  u64 start = i + 1;
  u64 n = line_vec->size - start;
  string8 instructions = {0};
  str_join(perm_arena, &instructions, STR8_LIT(""), n, lines + start);

  aoc_puzzle puzzle1 =
      (aoc_puzzle){.nx = nx,
                   .ny = ny,
                   .nobj = boxes->size,
                   .robotX = robotX,
                   .robotY = robotY,
                   .boxes = boxes->data,
                   .grid = grid->data,
                   .to_check = VEC_ARENA_CREATE(perm_arena, u32),
                   .to_move = VEC_ARENA_CREATE(perm_arena, u32)};
  aoc_puzzle puzzle2 = {0};
  puzzle_grow(&puzzle2, perm_arena, &puzzle1);

  u64 part1 = puzzle_solve(&puzzle1, instructions);
  printf("Part 1: %lu\n", part1);
  u64 part2 = puzzle_solve(&puzzle2, instructions);
  printf("Part 2: %lu\n", part2);

  arena_destroy(perm_arena);

  return EXIT_SUCCESS;
}

void parse_move(u8 move, i32 *dx, i32 *dy) {
  switch (move) {
  case '^':
    *dx = 0;
    *dy = -1;
    return;
  case '>':
    *dx = 1;
    *dy = 0;
    return;
  case 'v':
    *dx = 0;
    *dy = 1;
    return;
  case '<':
    *dx = -1;
    *dy = 0;
    return;
  }
  fprintf(stderr, "Unexpected move: '%c'\n", move);
  exit(1);
}

void puzzle_move(aoc_puzzle *puzzle, u8 move) {
  puzzle->to_check->size = 0;
  puzzle->to_move->size = 0;
  i32 dx = 0, dy = 0;
  parse_move(move, &dx, &dy);
  u32 x = puzzle->robotX + dx;
  u32 y = puzzle->robotY + dy;
  VEC_PUSH(puzzle->to_check, u32, y * puzzle->nx + x);
  while (puzzle->to_check->size > 0) {
    u32 indx = VEC_POP(puzzle->to_check, u32);
    i32 iobj = puzzle->grid[indx];
    if (iobj == -1) // wall
      return;
    if (iobj == 0 || vector_contains(puzzle->to_move, &iobj, eql))
      continue;
    VEC_PUSH(puzzle->to_move, u32, iobj);
    aoc_box box = puzzle->boxes[iobj - 1];
    if (dx == 0) {
      for (u32 w = 0; w < box.width; ++w) {
        u32 pixel = (box.yo + dy) * puzzle->nx + box.xo + w;
        VEC_PUSH(puzzle->to_check, u32, pixel);
      }
    } else if (dx == 1) {
      VEC_PUSH(puzzle->to_check, u32, box.yo * puzzle->nx + box.xo + box.width);
    } else {
      VEC_PUSH(puzzle->to_check, u32, box.yo * puzzle->nx + box.xo + dx);
    }
  }
  // We did not return before, so we can move
  u32 *iobjs = (u32 *)puzzle->to_move->data;
  for (u32 i = 0; i < puzzle->to_move->size; ++i) {
    aoc_box *box = puzzle->boxes + (iobjs[i] - 1);
    for (u32 w = 0; w < box->width; ++w) {
      u32 indx = box->yo * puzzle->nx + box->xo + w;
      puzzle->grid[indx] = 0;
    }
    box->xo += dx;
    box->yo += dy;
  }
  for (u32 i = 0; i < puzzle->to_move->size; ++i) {
    aoc_box *box = puzzle->boxes + (iobjs[i] - 1);
    for (u32 w = 0; w < box->width; ++w) {
      u32 indx = box->yo * puzzle->nx + box->xo + w;
      puzzle->grid[indx] = iobjs[i];
    }
  }
  puzzle->robotX += dx;
  puzzle->robotY += dy;
}

u64 puzzle_gps_score(aoc_puzzle *puzzle) {
  u64 score = 0;
  for (u32 i = 0; i < puzzle->nobj; ++i) {
    score += 100 * puzzle->boxes[i].yo + puzzle->boxes[i].xo;
  }
  return score;
}

u64 puzzle_solve(aoc_puzzle *puzzle, string8 instructions) {
  for (u32 i = 0; i < instructions.size; ++i) {
    puzzle_move(puzzle, instructions.str[i]);
  }
  puzzle_display(puzzle);
  return puzzle_gps_score(puzzle);
}

void puzzle_display(aoc_puzzle *puzzle) {
  for (u32 y = 0; y < puzzle->ny; ++y) {
    for (u32 x = 0; x < puzzle->nx; ++x) {
      i32 iobj = puzzle->grid[y * puzzle->nx + x];
      if (iobj == -1) {
        printf("#");
      } else if (iobj > 0) {
        aoc_box box = puzzle->boxes[iobj - 1];
        if (box.width == 1) {
          printf("O");
        } else if (x == box.xo) {
          printf("[");
        } else {
          printf("]");
        }
      } else if (x == puzzle->robotX && y == puzzle->robotY) {
        printf("@");
      } else {
        printf(".");
      }
    }
    printf("\n");
  }
}

void puzzle_grow(aoc_puzzle *dst, mem_arena *arena, const aoc_puzzle *src) {
  dst->ny = src->ny;
  dst->nx = src->nx * 2;
  dst->nobj = src->nobj;
  dst->grid = ALLOC_ARRAY(arena, i32, dst->nx * dst->ny);
  dst->boxes = ALLOC_ARRAY(arena, aoc_box, src->nobj);
  dst->robotX = 2 * src->robotX;
  dst->robotY = src->robotY;
  dst->to_check = VEC_ARENA_CREATE(arena, u32);
  dst->to_move = VEC_ARENA_CREATE(arena, u32);
  for (u32 y = 0; y < src->ny; ++y) {
    for (u32 x = 0; x < src->nx; ++x) {
      i32 iobj = src->grid[y * src->nx + x];
      dst->grid[y * dst->nx + 2 * x + 0] = iobj;
      dst->grid[y * dst->nx + 2 * x + 1] = iobj;
      if (iobj > 0) {
        dst->boxes[iobj - 1] = (aoc_box){.xo = 2 * x, .yo = y, .width = 2};
      }
    }
  }
}
