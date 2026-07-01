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
  plot_paper(&paper);

  free(file.str);
  vector_free(line_vec);
  hm_deinit(hm);

  return EXIT_SUCCESS;
}
