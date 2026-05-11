#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define DIM 3
#define KDTREE_IMPLEMENTATION
#include "kdtree.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

// #define HASHMAP_IMPLEMENTATION
// #include "hash_map.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
  u64 a, b;
  f64 dist;
} junction_t;

int junction_cmp(const void *a, const void *b) {
  return ((junction_t *)a)->dist - ((junction_t *)b)->dist;
}

int circuit_cmp(const void *a, const void *b) {
  vector *va = *(vector **)a;
  vector *vb = *(vector **)b;
  return vb->size - va->size;
}

void add_junction(u64 *n_circuit, vector **circuits, u64 a, u64 b) {
  u64 ia = *n_circuit, ib = *n_circuit;
  for (u64 i = 0; i < *n_circuit; ++i) {
    if (vector_contains(circuits[i], &a))
      ia = i;
    if (vector_contains(circuits[i], &b))
      ib = i;
  }
  if (ia == ib)
    return;
  u64 idst = MIN(ia, ib);
  u64 isrc = MAX(ia, ib);
  vector_extend(circuits[idst], circuits[isrc]);
  vector_free(circuits[isrc]);
  for (u64 i = isrc + 1; i < *n_circuit; ++i) {
    circuits[i - 1] = circuits[i];
  }
  circuits[(*n_circuit) - 1] = NULL;
  (*n_circuit)--;
}

void loadtxt(const char *filename, u64 *n, f64 **ptr) {
  string8 filecontent = {0};
  str_read_file(filename, &filecontent);
  vector *line_vec = VEC_CREATE(string8);
  split(line_vec, filecontent, STR8_LIT("\n"));
  *n = line_vec->size;
  *ptr = (f64 *)malloc(DIM * (*n) * sizeof(f64));
  f64 *xyz = *ptr;
  string8 *lines = (string8 *)line_vec->data;
  for (u64 i = 0; i < *n; ++i) {
    string8 line = lines[i];
    f64 v = 0;
    u64 k = 0;
    for (u64 j = 0; j <= line.size; ++j) {
      if (j == line.size || line.str[j] == ',') {
        xyz[DIM * i + k] = v;
        k++;
        v = 0;
      } else {
        v = v * 10 + (line.str[j] - '0');
      }
    }
  }
  vector_free(line_vec);
  free(filecontent.str);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  u64 npnts = 0;
  f64 *xyz = NULL;
  loadtxt(argv[1], &npnts, &xyz);

  vector *vec = VEC_CREATE(junction_t);
  for (u64 i = 0; i < npnts - 1; ++i) {
    for (u64 j = i + 1; j < npnts; ++j) {
      f64 d = compute_distance(xyz + DIM * i, xyz + DIM * j);
      junction_t pair = (junction_t){.a = MIN(i, j), .b = MAX(i, j), .dist = d};
      VEC_PUSH(vec, junction_t, pair);
    }
  }

  u64 n_pair = vec->size;
  junction_t *vec_pairs = vec->data;

  // kdtree tree = tree_from_points(npnts, xyz, 50);
  // f64 r = 0.5 * compute_distance(tree.root->xmin, tree.root->xmax);
  //
  // // Get unique pairs with distances
  // hash_map *hm_pairs = AUTO_HASHMAP(junction_t, u8);
  // u64 n, nalloc = 0;
  // f64 *distances = NULL;
  // u64 *indx = NULL;
  // u8 dummy = 0;
  //
  // for (u64 i = 0; i < npnts; ++i) {
  //   search_radius(&tree, xyz + DIM * i, r, &n, &nalloc, &indx, &distances);
  //   for (u64 j = 0; j < n; ++j) {
  //     if (i == indx[j])
  //       continue;
  //     junction_t pair = (junction_t){
  //         .a = MIN(i, indx[j]), .b = MAX(i, indx[j]), .dist = distances[j]};
  //     hm_put(hm_pairs, &pair, &dummy);
  //   }
  // }
  // tree_free(&tree);
  // free(distances);
  // free(indx);

  // Sort pairs
  // u64 n_pair = hm_pairs->size;
  // junction_t *vec_pairs = (junction_t *)malloc(sizeof(junction_t) * n_pair);
  // kv_iterator kvi = hm_iterator(hm_pairs);
  // u64 ijun = 0;
  // while (get_next(&kvi)) {
  //   vec_pairs[ijun++] = *((junction_t *)kvi.key_ptr);
  // }
  // hm_deinit(hm_pairs);
  qsort(vec_pairs, n_pair, sizeof(junction_t), junction_cmp);

  // Initialize circuits
  u64 n_circuit = npnts;
  vector **circuits = (vector **)malloc(sizeof(vector *) * n_circuit);
  for (u64 i = 0; i < n_circuit; ++i) {
    circuits[i] = VEC_CREATE(u64);
    VEC_PUSH(circuits[i], u64, i);
  }

  u64 n_loop = npnts < 100 ? 10 : 1000;
  u64 iloop;
  for (iloop = 0; iloop < n_loop; ++iloop) {
    junction_t jun = vec_pairs[iloop];
    add_junction(&n_circuit, circuits, jun.a, jun.b);
  }

  qsort(circuits, n_circuit, sizeof(vector *), circuit_cmp);
  u64 part1 = circuits[0]->size * circuits[1]->size * circuits[2]->size;

  printf("Part 1: %lu\n", part1);

  u64 part2;
  while (iloop < n_pair && n_circuit > 1) {
    junction_t jun = vec_pairs[iloop++];
    add_junction(&n_circuit, circuits, jun.a, jun.b);
    f64 *xa = xyz + DIM * jun.a;
    f64 *xb = xyz + DIM * jun.b;
    part2 = xa[0] * xb[0];
  }

  printf("Part 2: %lu\n", (u64)part2);

  free(xyz);
  // free(vec_pairs);
  vector_free(vec);
  for (u64 i = 0; i < n_circuit; ++i) {
    vector_free(circuits[i]);
  }
  free(circuits);

  return EXIT_SUCCESS;
}
