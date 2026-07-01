#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#define HASHMAP_IMPLEMENTATION
#include "hash_map.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
  u64 nx, ny;
  hash_map *dots;
} folded_paper;

void plot_paper(folded_paper *paper) {
  u64 pos[2];
  kv_entry entry;
  for (u64 y = 0; y < paper->ny; ++y) {
    for (u64 x = 0; x < paper->nx; ++x) {
      pos[0] = x;
      pos[1] = y;
      entry = hm_get_entry(paper->dots, &pos);
      printf(entry.found_existing ? "#" : ".");
    }
    printf("\n");
  }
  printf("\n");
}

void fold_x(folded_paper *paper, u64 x_fold) {
  assert(x_fold == paper->nx / 2);
  u64 *keys = (u64 *)hm_keys(paper->dots);
  u64 pos[2], dx;
  u8 dummy = 1;
  u64 n = paper->dots->size;
  hm_reset(paper->dots);
  for (u64 i = 0; i < n; ++i) {
    pos[0] = keys[2 * i + 0];
    pos[1] = keys[2 * i + 1];
    if (pos[0] > x_fold) {
      dx = pos[0] - x_fold;
      pos[0] = x_fold - dx;
    }
    hm_put(paper->dots, &pos, &dummy);
  }
  free(keys);
  paper->nx = x_fold;
}

void fold_y(folded_paper *paper, u64 y_fold) {
  assert(y_fold == paper->ny / 2);
  u64 *keys = (u64 *)hm_keys(paper->dots);
  u64 pos[2], dy;
  u8 dummy = 1;
  u64 n = paper->dots->size;
  hm_reset(paper->dots);
  for (u64 i = 0; i < n; ++i) {
    pos[0] = keys[2 * i + 0];
    pos[1] = keys[2 * i + 1];
    if (pos[1] > y_fold) {
      dy = pos[1] - y_fold;
      pos[1] = y_fold - dy;
    }
    hm_put(paper->dots, &pos, &dummy);
  }
  free(keys);
  paper->ny = y_fold;
}

void fold_paper(folded_paper *paper, vector *word_vec,
                string8 fold_instruction) {
  string8 splitted[2] = {0};
  u64 ax_pos;
  split_whitespace(word_vec, fold_instruction);
  string8 last_word = *(string8 *)vector_get_last(word_vec);
  str_split_once(splitted, last_word, STR8_LIT("="));
  str_parse_unsigned(&ax_pos, splitted[1]);
  if (str_equal(splitted[0], STR8_LIT("x")))
    fold_x(paper, ax_pos);
  else
    fold_y(paper, ax_pos);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);
  vector *line_vec = VEC_CREATE(string8);
  split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;
  u64 nx = 0, ny = 0;
  hash_map *hm = AUTO_HASHMAP(u64[2], u8);
  string8 splitted[2] = {0};
  string8 delim = STR8_LIT(",");
  u64 x, y;
  u64 pos[2];
  u8 dummy = 1;
  u64 iln;
  for (iln = 0; iln < line_vec->size && lines[iln].size > 0; ++iln) {
    str_split_once(splitted, lines[iln], delim);
    str_parse_unsigned(&x, splitted[0]);
    str_parse_unsigned(&y, splitted[1]);
    nx = MAX(nx, x + 1);
    ny = MAX(ny, y + 1);
    pos[0] = x;
    pos[1] = y;
    hm_put(hm, &pos, &dummy);
  }
  folded_paper paper = (folded_paper){.nx = nx, .ny = ny, .dots = hm};

  vector *word_vec = VEC_CREATE(string8);
  fold_paper(&paper, word_vec, lines[++iln]);

  printf("Part 1: %lu\n", hm->size);
  for (iln = iln + 1; iln < line_vec->size; ++iln) {
    fold_paper(&paper, word_vec, lines[iln]);
  }
  plot_paper(&paper);

  vector_free(word_vec);
  vector_free(line_vec);
  hm_deinit(hm);
  free(file.str);

  return EXIT_SUCCESS;
}
