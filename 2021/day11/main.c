#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#define QUEUE_IMPLEMENTATION
#include "queue.h"

typedef struct {
  u64 ny, nx;
  u8 *data;
  queue *queue;
  b8 *flashed;
} octogrid;

u32 iterate(octogrid *grid) {
  u64 y, x, index;
  memset(grid->flashed, 0, grid->nx * grid->ny);
  for (y = 0; y < grid->ny; ++y) {
    for (x = 0; x < grid->nx; ++x) {
      index = y * grid->nx + x;
      if (++grid->data[index] > 9)
        QUEUE_PUSH(grid->queue, u64, index);
    }
  }
  u64 xmin, xmax, ymin, ymax;
  u64 x2, y2, index2;
  while (grid->queue->size > 0) {
    index = QUEUE_POP(grid->queue, u64);
    grid->flashed[index] = 1;
    x = index % grid->nx;
    y = index / grid->nx;
    xmin = x > 0 ? x - 1 : 0;
    xmax = MIN(x + 1, grid->nx - 1);
    ymin = y > 0 ? y - 1 : 0;
    ymax = MIN(y + 1, grid->ny - 1);
    for (y2 = ymin; y2 <= ymax; ++y2) {
      for (x2 = xmin; x2 <= xmax; ++x2) {
        index2 = grid->nx * y2 + x2;
        if (index == index2 || grid->flashed[index2] ||
            queue_contains(grid->queue, &index2))
          continue;
        if (++grid->data[index2] > 9)
          QUEUE_PUSH(grid->queue, u64, index2);
      }
    }
  }
  u32 cntr = 0;
  for (y = 0; y < grid->ny; ++y) {
    for (x = 0; x < grid->nx; ++x) {
      index = y * grid->nx + x;
      if (!grid->flashed[index])
        continue;
      grid->data[index] = 0;
      cntr++;
    }
  }
  return cntr;
}

void print_grid(octogrid *grid) {
  printf("\n");
  u8 v;
  for (u64 y = 0; y < grid->ny; ++y) {
    for (u64 x = 0; x < grid->nx; ++x) {
      v = grid->data[y * grid->nx + x];
      printf(v == 0 ? "\x1b[31m%hu\x1b[0m" : "%hu", v);
    }
    printf("\n");
  }
  printf("\n");
}

octogrid read_input_file(const char *filename) {
  string8 file = {0};
  str_read_file(NULL, &file, filename);
  vector *line_vec = VEC_CREATE(string8);
  split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;
  u64 ny = line_vec->size;
  u8 *data = NULL;
  u64 j, k = 0, nx = 0;
  for (u64 i = 0; i < ny; ++i) {
    if (data == NULL) {
      nx = lines[i].size;
      data = (u8 *)malloc(ny * nx);
    }
    for (j = 0; j < nx; ++j) {
      data[k++] = lines[i].str[j] - '0';
    }
  }
  free(file.str);
  vector_free(line_vec);
  queue *q = QUEUE_CREATE(u64);
  return (octogrid){.ny = ny,
                    .nx = nx,
                    .data = data,
                    .queue = q,
                    .flashed = (b8 *)malloc(nx * ny)};
}

int main(int argc, char *argv[]) {
  if (argc == 0) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  octogrid grid = read_input_file(argv[1]);
  u64 part1 = 0; 
  u64 part2 = 0;
  u32 n_flashed = 0;
  u64 i;
  for (i = 0; i < 100; ++i) {
    n_flashed = iterate(&grid);
    part1 += n_flashed;
    if (n_flashed == grid.nx * grid.ny) {
      part2 = i;
    }
  }

  printf("Part 1 = %lu\n", part1);

  while (part2 == 0) {
    i++;
    n_flashed = iterate(&grid);
    if (n_flashed == (grid.nx * grid.ny)) {
      part2 = i;
    }
  }

  printf("Part 2 = %lu\n", part2);

  free(grid.data);
  free(grid.flashed);
  queue_free(grid.queue);
  return EXIT_SUCCESS;
}
