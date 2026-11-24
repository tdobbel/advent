#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_IMPLEMENTATION
#include "string8.h"
#define HASHMAP_IMPLEMENTATION
#include "hash_map.h"

#define WIDTH 7
#define MEM_BUF_SIZE 64
#define N_SHAPE 5
#define X0 2
#define SPAWN_HEIGHT 4
#define N_ITER_PART2 1000000000000
#define N_ROW_STATE 30

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef enum { Right, Down, Left } Direction;

typedef struct {
  u32 width, height;
  b8 *mask;
} Shape;

typedef struct {
  u32 x, y;
} Position;

typedef struct {
  Position ll;
  const Shape *shape;
} Rock;

typedef struct {
  u32 idx0;
  u64 y0;
  u32 n_row;
  u32 highest; // relative to idx0
  u8 buf[MEM_BUF_SIZE][WIDTH];
} MemoryBuffer;

typedef struct {
  u32 shape, move;
  u8 buf[N_ROW_STATE * WIDTH];
} State;

typedef struct {
  u64 iter, height;
} Timestamp;

Shape shapes[N_SHAPE] = {
    (Shape){.width = 4, .height = 1, .mask = (b8[4]){1, 1, 1, 1}},
    (Shape){
        .width = 3,
        .height = 3,
        .mask = (b8[9]){0, 1, 0, 1, 1, 1, 0, 1, 0},
    },
    (Shape){
        .width = 3, .height = 3, .mask = (b8[9]){1, 1, 1, 0, 0, 1, 0, 0, 1}},
    (Shape){.width = 1, .height = 4, .mask = (b8[4]){1, 1, 1, 1}},
    (Shape){.width = 2, .height = 2, .mask = (b8[4]){1, 1, 1, 1}}};

Rock rock_init(const Shape *shape, MemoryBuffer *mb);
b8 rock_move(Rock *rock, Direction d, MemoryBuffer *mb);
void memory_buffer_reset(MemoryBuffer *mb);
void get_last_rows(u8 buf[N_ROW_STATE * WIDTH], const MemoryBuffer *mb);
void rock_fall(Rock *rock, string8 jets, u32 *iptr, MemoryBuffer *mb);
void debug_display(const Rock *rock, MemoryBuffer *mb);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);
  file = str_trim(file);

  MemoryBuffer mb = {0};

  u32 iptr = 0;
  u64 iter;
  for (iter = 0; iter < 2022; ++iter) {
    Rock rock = rock_init(&shapes[iter % N_SHAPE], &mb);
    rock_fall(&rock, file, &iptr, &mb);
  }

  printf("Part 1 = %lu\n", mb.y0 + mb.highest);

  hash_map *states = AUTO_HASHMAP(State, Timestamp);
  u8 buf[N_ROW_STATE * WIDTH];
  memory_buffer_reset(&mb);
  iter = 0;
  iptr = 0;
  b8 cycle_found = false;
  u64 extra_h = 0;
  while (iter < N_ITER_PART2) {
    u32 ishp = iter % N_SHAPE;
    Rock rock = rock_init(&shapes[iter % N_SHAPE], &mb);
    rock_fall(&rock, file, &iptr, &mb);
    iter++;
    if (!cycle_found && mb.highest >= N_ROW_STATE - 1) {
      get_last_rows(buf, &mb);
      State state = (State){.move = iptr, .shape = ishp};
      memcpy(state.buf, buf, N_ROW_STATE * WIDTH);
      Timestamp tt =
          (Timestamp){.iter = iter, .height = (u64)mb.highest + mb.y0};
      kv_entry entry = hm_get_or_put(states, &state);
      if (entry.found_existing) {
        Timestamp prev = *(Timestamp *)entry.value_ptr;
        u64 period = tt.iter - prev.iter;
        u64 dh = tt.height - prev.height;
        u64 n_cycle = (N_ITER_PART2 - iter) / period;
        iter += n_cycle * period;
        extra_h = n_cycle * dh;
        cycle_found = true;
      } else {
        *(Timestamp *)entry.value_ptr = tt;
      }
    }
  }

  u64 part2 = extra_h + mb.highest + mb.y0;
  printf("Part 2 = %lu\n", part2);

  hm_deinit(states);
  free(file.str);

  return EXIT_SUCCESS;
}

Rock rock_init(const Shape *shape, MemoryBuffer *mb) {
  for (u32 i = 0; i < 8; ++i) {
    u32 n = mb->highest + i;
    if (n < mb->n_row)
      continue;
    u32 r = (mb->idx0 + n) % MEM_BUF_SIZE;
    memset(mb->buf[r], '.', WIDTH);
    if (mb->n_row < MEM_BUF_SIZE) {
      mb->n_row++;
    } else {
      mb->highest--;
      mb->idx0++;
      mb->y0++;
    }
  }
  Position ll = (Position){.x = X0, .y = mb->highest + SPAWN_HEIGHT};
  return (Rock){.ll = ll, .shape = shape};
}

b8 rock_move(Rock *rock, Direction d, MemoryBuffer *mb) {
  Position new_pos = rock->ll;
  const Shape *shp = rock->shape;
  switch (d) {
  case Left: {
    if (new_pos.x == 0)
      return false;
    new_pos.x--;
  } break;
  case Right: {
    if (new_pos.x + shp->width == WIDTH)
      return false;
    new_pos.x++;
  } break;
  case Down: {
    if (new_pos.y == 1) {
      assert(mb->y0 == 0);
      return false;
    }
    new_pos.y--;
  } break;
  }
  for (u32 iy = 0; iy < shp->height; ++iy) {
    u32 y = new_pos.y + iy;
    if (y >= mb->n_row)
      continue; // We are outside of the buffer
    for (u32 ix = 0; ix < shp->width; ++ix) {
      if (!rock->shape->mask[iy * shp->width + ix])
        continue;
      u32 x = new_pos.x + ix;
      u32 r = (mb->idx0 + y) % MEM_BUF_SIZE;
      if (mb->buf[r][x] == '#')
        return false;
    }
  }
  rock->ll = new_pos;
  return true;
}

void rock_fall(Rock *rock, string8 jets, u32 *iptr, MemoryBuffer *mb) {
  // debug_display(rock, mb);
  while (true) {
    Direction d = jets.str[*iptr] == '>' ? Right : Left;
    rock_move(rock, d, mb);
    *iptr = (*iptr + 1) % jets.size;
    if (!rock_move(rock, Down, mb))
      break;
    // debug_display(rock, mb);
  }
  // debug_display(rock, mb);
  const Shape *shp = rock->shape;
  for (u32 iy = 0; iy < shp->height; ++iy) {
    u32 y = rock->ll.y + iy;
    u32 r = (mb->idx0 + y) % MEM_BUF_SIZE;
    for (u32 ix = 0; ix < shp->width; ++ix) {
      if (shp->mask[iy * shp->width + ix]) {
        u32 x = rock->ll.x + ix;
        mb->buf[r][x] = '#';
        mb->highest = MAX(mb->highest, y);
      }
    }
  }
}

void debug_display(const Rock *rock, MemoryBuffer *mb) {
  for (u32 y = mb->n_row; y > 0; --y) {
    u32 y2 = y - 1;
    const Shape *shp = rock->shape;
    for (u32 x = 0; x < WIDTH; ++x) {
      if (y2 == 0 && mb->y0 == 0) {
        printf("-");
        continue;
      }
      if (y2 >= rock->ll.y && y2 < rock->ll.y + shp->height &&
          x >= rock->ll.x && x < rock->ll.x + shp->width) {
        u32 ix = x - rock->ll.x;
        u32 iy = y2 - rock->ll.y;
        if (shp->mask[iy * shp->width + ix]) {
          printf("@");
          continue;
        }
      }
      u32 r = (mb->idx0 + y2) % MEM_BUF_SIZE;
      printf("%c", mb->buf[r][x]);
    }
    printf("\n");
  }
  printf("\n");
  printf("highest=%u\n", mb->highest);
}

void memory_buffer_reset(MemoryBuffer *mb) {
  mb->y0 = 0;
  mb->highest = 0;
  mb->idx0 = 0;
  mb->n_row = 0;
}

void get_last_rows(u8 buf[N_ROW_STATE * WIDTH], const MemoryBuffer *mb) {
  for (u32 i = 0; i < N_ROW_STATE; ++i) {
    u32 y = mb->highest - i;
    u32 r = (mb->idx0 + y) % MEM_BUF_SIZE;
    for (u32 x = 0; x < WIDTH; ++x) {
      buf[i * WIDTH + x] = mb->buf[r][x];
    }
  }
}
