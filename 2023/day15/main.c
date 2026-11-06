#include "stdio.h"
#include "stdlib.h"

#define ARENA_IMPLEMENTATION
#define STRING_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION

#include "arena.h"
#include "string8.h"
#include "vector.h"

#define N_BOX 256

u32 hash(string8 input);

typedef struct {
  string8 label;
  u32 focal_length;
} Lens;

b8 box_contains(vector *box, string8 label);
void box_remove_label(vector *box, string8 label);
void box_add_lens(vector *box, Lens lens);
void box_print(vector **boxes);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  mem_arena *arena = arena_create(KiB(500));

  string8 file = {0};
  str_read_file(arena, &file, argv[1]);
  string8 trimmed = str_trim(file);

  vector *vec = VEC_ARENA_CREATE(arena, string8);
  str_split(vec, trimmed, STR8_LIT(","));
  string8 *ops = (string8 *)vec->data;

  u32 part1 = 0;
  for (u32 i = 0; i < vec->size; ++i) {
    part1 += hash(ops[i]);
  }

  printf("Part 1: %u\n", part1);
  vector **boxes = ALLOC_ARRAY(arena, vector *, N_BOX);
  for (u32 i = 0; i < N_BOX; ++i) {
    boxes[i] = VEC_ARENA_CREATE(arena, Lens);
  }

  // Part 2
  string8 equal_sign = STR8_LIT("=");
  string8 splitted[2] = {0};
  u32 focal_length, ibox;

  for (u64 iop = 0; iop < vec->size; ++iop) {
    string8 op = ops[iop];
    // printf("After \"" STR8_FMT "\":\n", STR8_UNWRAP(op));
    if (op.str[op.size - 1] == '-') {
      string8 label = (string8){.str = op.str, .size = op.size - 1};
      ibox = hash(label);
      box_remove_label(boxes[ibox], label);
    } else {
      str_split_once(splitted, op, equal_sign);
      focal_length = str_parse_unsigned(splitted[1]);
      ibox = hash(splitted[0]);
      Lens lens = (Lens){.label = splitted[0], .focal_length = focal_length};
      box_add_lens(boxes[ibox], lens);
    }
    // box_print(boxes);
  }

  u32 part2 = 0;
  for (ibox = 0; ibox < N_BOX; ++ibox) {
    Lens *lenses = (Lens *)boxes[ibox]->data;
    for (u32 i = 0; i < boxes[ibox]->size; ++i) {
      part2 += (ibox + 1) * (i + 1) * lenses[i].focal_length;
    }
  }
  printf("Part 2: %u\n", part2);
  arena_destroy(arena);

  return EXIT_SUCCESS;
}

u32 hash(string8 input) {
  u32 r = 0;
  for (u64 ic = 0; ic < input.size; ++ic) {
    r += (u32)input.str[ic];
    r *= 17;
    r %= N_BOX;
  }
  return r;
}

b8 box_contains(vector *box, string8 label) {
  Lens *lenses = (Lens *)box->data;
  for (u32 i = 0; i < box->size; ++i) {
    if (str_equal(lenses[i].label, label))
      return 1;
  }
  return 0;
}

void box_remove_label(vector *box, string8 label) {
  Lens *lenses = (Lens *)box->data;
  u32 n = 0;
  for (u32 i = 0; i < box->size; ++i) {
    if (str_equal(lenses[i].label, label))
      continue;
    if (n < i) {
      lenses[n] = lenses[i];
    }
    n++;
  }
  box->size = n;
}

void box_add_lens(vector *box, Lens lens) {
  Lens *lenses = (Lens *)box->data;
  for (u32 i = 0; i < box->size; ++i) {
    if (str_equal(lenses[i].label, lens.label)) {
      lenses[i].focal_length = lens.focal_length;
      return;
    }
  }
  VEC_PUSH(box, Lens, lens);
}

void box_print(vector **boxes) {
  for (u32 ibox = 0; ibox < N_BOX; ibox++) {
    if (boxes[ibox]->size == 0)
      continue;
    printf("Box %u: ", ibox);
    Lens *lenses = (Lens *)boxes[ibox]->data;
    for (u32 i = 0; i < boxes[ibox]->size; ++i) {
      printf("[" STR8_FMT " %u]", STR8_UNWRAP(lenses[i].label),
             lenses[i].focal_length);
      if (i < boxes[ibox]->size - 1)
        printf(" ");
    }
    printf("\n");
  }
}
