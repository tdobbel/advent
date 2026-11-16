#include <stdint.h>
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

typedef struct {
  u32 ny, nx;
  i32 xmin;
  u8 *data;
} Grid;

typedef struct {
  i32 x, y;
} Point;

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define GRID(g, x, y) (g)->data[(y) * (g)->nx + (x)]

void add_wall(Grid *grid, hash_map *used, Point *wall, u64 npnts);
b8 add_sand_part1(Grid *grid);
b8 add_sand_part2(Grid *grid, hash_map *used);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  mem_arena *gpa = arena_create(KiB(500));

  i32 xmin = INT32_MAX;
  i32 xmax = 0, ymax = 0;
  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);

  vector *line_vec = VEC_CREATE(string8);
  str_split(line_vec, str_trim(file), STR8_LIT("\n"));
  string8 *lines = VEC_CAST(line_vec, string8);

  vector *wall_vec = VEC_CREATE(vector *);
  vector *point_vec = VEC_CREATE(string8);
  string8 splitted[2];

  for (u32 i = 0; i < line_vec->size; ++i) {
    str_split(point_vec, lines[i], STR8_LIT(" -> "));
    string8 *point_str = VEC_CAST(point_vec, string8);
    vector *wall = VEC_ARENA_CREATE(gpa, Point);
    for (u32 ip = 0; ip < point_vec->size; ++ip) {
      str_split_once(splitted, point_str[ip], STR8_LIT(","));
      i32 x = str_parse_unsigned(splitted[0]);
      i32 y = str_parse_unsigned(splitted[1]);
      Point point = (Point){.x = x, .y = y};
      xmin = MIN(xmin, x);
      xmax = MAX(xmax, x);
      ymax = MAX(ymax, y);
      vector_push(wall, &point);
    }
    vector_push(wall_vec, &wall);
  }

  vector_deinit(point_vec);
  vector_deinit(line_vec);
  free(file.str);

  u32 nx = xmax - xmin + 1;
  u32 ny = ymax + 1;

  u8 *data = ALLOC_ARRAY(gpa, u8, nx * ny);
  memset(data, '.', nx * ny);
  Grid grid = (Grid){.ny = ny, .nx = nx, .xmin = xmin, .data = data};

  hash_map *used = AUTO_HASHMAP(Point, b8);
  vector **walls = VEC_CAST(wall_vec, vector *);
  for (u32 i = 0; i < wall_vec->size; ++i) {
    Point *wall = VEC_CAST(walls[i], Point);
    add_wall(&grid, used, wall, walls[i]->size);
  }

  u32 part1 = 0;
  while (add_sand_part1(&grid)) {
    part1++;
  }
  printf("Part 1: %u\n", part1);

  u32 part2 = 0;
  while(add_sand_part2(&grid, used)) {
    part2++;
  }
  printf("Part 2: %u\n", part2 + 1);

  hm_deinit(used);
  vector_deinit(wall_vec);
  arena_destroy(gpa);

  return EXIT_SUCCESS;
}

void add_wall(Grid *grid, hash_map *used, Point *wall, u64 npnts) {
  b8 dummy = 1;
  for (u32 i = 0; i < npnts - 1; ++i) {
    Point p0 = wall[i];
    Point p1 = wall[i + 1];
    if (p0.x == p1.x) {
      i32 startY = MIN(p0.y, p1.y);
      i32 endY = MAX(p0.y, p1.y);
      for (i32 y = startY; y <= endY; ++y) {
        GRID(grid, p0.x - grid->xmin, y) = '#';
        Point p = (Point){.x = p0.x, .y = y};
        hm_put(used, &p, &dummy);
      }
    } else {
      i32 startX = MIN(p0.x, p1.x);
      i32 endX = MAX(p0.x, p1.x);
      for (i32 x = startX; x <= endX; ++x) {
        GRID(grid, x - grid->xmin, p0.y) = '#';
        Point p = (Point){.x = x, .y = p0.y};
        hm_put(used, &p, &dummy);
      }
    }
  }
}

b8 add_sand_part1(Grid *grid) {
  u32 x = 500 - grid->xmin;
  u32 y = 0;
  while (true) {
    if (y + 1 == grid->ny)
      return false;
    if (GRID(grid, x, y + 1) == '.') {
      y++;
      continue;
    }
    if (x == 0)
      return false;
    if (GRID(grid, x - 1, y + 1) == '.') {
      y++;
      x--;
      continue;
    }
    if (x == grid->nx - 1)
      return false;
    if (GRID(grid, x + 1, y + 1) == '.') {
      y++;
      x++;
      continue;
    }
    GRID(grid, x, y) = 'o';
    return true;
  }
}

b8 add_sand_part2(Grid *grid, hash_map *used) {
  i32 xo = 500;
  i32 yo = 0;
  Point pos = (Point){.x = xo, .y = yo};
  i32 ground_level = grid->ny + 1;
  b8 moved = false;
  while (true) {
    Point next_pos = (Point){.x = pos.x, .y = pos.y};
    if (pos.y + 1 == ground_level) {
      hm_put(used, &pos, &moved);
      return moved;
    }
    next_pos.y++;
    if (!hm_contains_key(used, &next_pos)) {
      pos = next_pos;
      moved = true;
      continue;
    }
    next_pos.x = pos.x - 1;
    if (!hm_contains_key(used, &next_pos)) {
      pos = next_pos;
      moved = true;
      continue;
    }
    next_pos.x = pos.x + 1;
    if (!hm_contains_key(used, &next_pos)) {
      pos = next_pos;
      moved = true;
      continue;
    }
    hm_put(used, &pos, &moved);
    return moved;
  }
}
