#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION

#include "string8.h"
#include "vector.h"

#include "arena.h"

#define ABSDIFF(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))

typedef struct {
  u32 x, y;
} Position;

typedef struct {
  u32 x, y, length;
} Path;

typedef struct {
  u32 ny, nx;
  u8 *grid;
  u32 *shortest;
  u32 *previous;
} Maze;

Order compare_paths(const void *ctx, const void *a, const void *b);
void find_shortest(Maze *maze, Path path);
u32 count_shortcuts(Position *path, u32 path_size, u32 min_save, u32 n_cheat);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  mem_arena *gpa = arena_create(KiB(500));
  string8 file = {0};

  str_read_file(gpa, &file, argv[1]);

  vector *line_vec = VEC_CREATE(string8);
  str_split(line_vec, str_trim(file), STR8_LIT("\n"));
  string8 *lines = VEC_CAST(line_vec, string8);
  u32 ny = line_vec->size;
  u32 nx = lines[0].size;

  u8 *grid = ALLOC_ARRAY(gpa, u8, nx * ny);
  u32 startX = nx, startY = ny;
  u32 endX = nx, endY = ny;
  u32 px = 0;
  for (u32 y = 0; y < ny; ++y) {
    for (u32 x = 0; x < nx; ++x) {
      u8 c = lines[y].str[x];
      grid[px++] = c;
      if (c == 'S') {
        startX = x;
        startY = y;
      }
      if (c == 'E') {
        endX = x;
        endY = y;
      }
    }
  }

  vector_deinit(line_vec);

  u32 *prev = ALLOC_ARRAY(gpa, u32, nx * ny);
  u32 *shortest = ALLOC_ARRAY(gpa, u32, nx * ny);
  for (px = 0; px < nx * ny; ++px) {
    prev[px] = nx * ny;
    shortest[px] = 0xffffffff;
  }

  Maze maze = (Maze){
      .nx = nx, .ny = ny, .grid = grid, .shortest = shortest, .previous = prev};

  shortest[startY * nx + startX] = 0;
  Path path = (Path){.x = startX, .y = startY, .length = 0};

  find_shortest(&maze, path);

  px = endY * nx + endX;
  u32 path_size = shortest[px] + 1;
  Position *path_pos = ALLOC_ARRAY(gpa, Position, path_size);
  for (u32 i = path_size; i > 0; --i) {
    path_pos[i - 1] = (Position){.x = px % nx, .y = px / nx};
    px = prev[px];
  }

  u32 part1 = count_shortcuts(path_pos, path_size, 100, 2);
  printf("Part 1: %u\n", part1);
  u32 part2 = count_shortcuts(path_pos, path_size, 100, 20);
  printf("Part 2: %u\n", part2);

  arena_destroy(gpa);

  return EXIT_SUCCESS;
}

Order compare_paths(const void *ctx, const void *a, const void *b) {
  Maze *maze = (Maze *)ctx;
  Path *pathA = (Path *)a;
  Path *pathB = (Path *)b;
  u32 scoreA =
      pathA->length + (maze->nx - pathA->x - 1) + (maze->ny - pathA->y - 1);
  u32 scoreB =
      pathB->length + (maze->nx - pathB->x - 1) + (maze->ny - pathB->y - 1);
  if (scoreA < scoreB)
    return lt;
  if (scoreA < scoreB)
    return gt;
  return eq;
}

void find_shortest(Maze *maze, Path path) {
  PriorityQueue *queue = PRIORITY_QUEUE_CREATE(Path, maze, compare_paths);
  priority_queue_push(queue, &path);
  Path p;
  u32 next[4] = {0};
  u32 n_next;
  while (priority_queue_pop(queue, &p)) {
    u32 curr = p.y * maze->nx + p.x;
    if (maze->grid[curr] == 'E')
      continue;
    n_next = 0;
    if (p.x > 0)
      next[n_next++] = (p.y + 0) * maze->nx + p.x - 1;
    if (p.y > 0)
      next[n_next++] = (p.y - 1) * maze->nx + p.x + 0;
    if (p.x < maze->nx - 1)
      next[n_next++] = (p.y + 0) * maze->nx + p.x + 1;
    if (p.y < maze->ny - 1)
      next[n_next++] = (p.y + 1) * maze->nx + p.x + 0;
    for (u32 k = 0; k < n_next; ++k) {
      u32 px = next[k];
      if (maze->grid[px] == '#')
        continue;
      if (!(maze->shortest[px] > p.length + 1))
        continue;
      maze->shortest[px] = p.length + 1;
      maze->previous[px] = curr;
      Path next_path = (Path){
          .x = px % maze->nx, .y = px / maze->nx, .length = p.length + 1};
      priority_queue_push(queue, &next_path);
    }
  }
  priority_queue_deinit(queue);
}

u32 count_shortcuts(Position *path, u32 path_size, u32 min_save, u32 n_cheat) {
  u32 total = 0;
  for (u32 start = 0; start < path_size - min_save; ++start) {
    for (u32 stop = start + min_save + 1; stop < path_size; ++stop) {
      u32 distance = ABSDIFF(path[stop].x, path[start].x) +
                     ABSDIFF(path[stop].y, path[start].y);
      if (distance > n_cheat)
        continue;
      u32 save = stop - start - distance;
      if (save >= min_save)
        total++;
    }
  }
  return total;
}
