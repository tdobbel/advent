#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#define HASHMAP_IMPLEMENTATION
#include "hash_map.h"

typedef struct {
  u32 a, b, c;
} triplet;

b8 str_eql_fun(const void *a, const void *b) {
  return str_equal(*(string8 *)a, *(string8 *)b);
}

b8 u32_eql_fun(const void *a, const void *b) { return *(u32 *)a == *(u32 *)b; }

int cmp_fn(const void *a, const void *b) { return *(u32 *)a - *(u32 *)b; }

int str8_cmp(const void *a, const void *b) {
  string8 sa = *(string8 *)a;
  string8 sb = *(string8 *)b;
  u64 n = MIN(sa.size, sb.size);
  for (u64 i = 0; i < n; ++i) {
    if (sa.str[i] > sb.str[i])
      return 1;
    if (sa.str[i] < sb.str[i])
      return -1;
  }
  return 0;
}

u32 get_name_index(vector *names, string8 name);
void add_neighbor(mem_arena *arena, hash_map *adj, u32 nfrom, u32 nto);
void print_adjacency(hash_map *adj, string8 *names);
u32 count_triangles(hash_map *adj, string8 *names);
string8 find_max_clique(mem_arena *arena, hash_map *adj, vector *name_vec);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  mem_arena *perm_arena = arena_create(MiB(10));
  string8 file = {0};
  str_read_file(perm_arena, &file, argv[1]);
  vector *line_vec = VEC_ARENA_CREATE(perm_arena, string8);
  split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;

  hash_map *adj = AUTO_HASHMAP_ARENA(perm_arena, u32, vector *);
  vector *names = VEC_ARENA_CREATE(perm_arena, string8);

  string8 splitted[2] = {0};
  string8 sep = STR8_LIT("-");
  for (u64 i = 0; i < line_vec->size && lines[i].size > 0; ++i) {
    str_split_once(splitted, lines[i], sep);
    u32 indx1 = get_name_index(names, splitted[0]);
    u32 indx2 = get_name_index(names, splitted[1]);
    add_neighbor(perm_arena, adj, indx1, indx2);
    add_neighbor(perm_arena, adj, indx2, indx1);
  }

  // print_adjacency(adj, names->data);
  u32 part1 = count_triangles(adj, names->data);
  printf("Part 1: %u\n", part1);

  string8 max_clique = find_max_clique(perm_arena, adj, names);
  printf("Part 2: " STR8_FMT "\n", STR8_UNWRAP(max_clique));

  arena_destroy(perm_arena);
  return EXIT_SUCCESS;
}

u32 get_name_index(vector *names, string8 name) {
  u64 indx = vector_get_index(names, &name, str_eql_fun);
  if (indx == names->size)
    VEC_PUSH(names, string8, name);
  return indx;
}

void add_neighbor(mem_arena *arena, hash_map *adj, u32 nfrom, u32 nto) {
  kv_entry entry = hm_get_or_put(adj, &nfrom);
  vector **vp = (vector **)entry.value_ptr;
  if (!entry.found_existing) {
    *vp = VEC_ARENA_CREATE(arena, u32);
  }
  VEC_PUSH(*vp, u32, nto);
}

void print_adjacency(hash_map *adj, string8 *names) {
  kv_iterator kvi = hm_iterator(adj);
  while (get_next(&kvi)) {
    string8 src = names[*(u32 *)kvi.key_ptr];
    vector *vp = *(vector **)kvi.value_ptr;
    u32 *inodes = (u32 *)vp->data;
    printf(STR8_FMT " => ", STR8_UNWRAP(src));
    for (u64 i = 0; i < vp->size; ++i) {
      printf(STR8_FMT " ", STR8_UNWRAP(names[inodes[i]]));
    }
    printf("\n");
  }
}

u32 count_triangles(hash_map *adj, string8 *names) {
  hash_map *tris = AUTO_HASHMAP(triplet, b8);
  b8 dummy = 1;
  kv_iterator kvi = hm_iterator(adj);
  while (get_next(&kvi)) {
    u32 node0 = *(u32 *)kvi.key_ptr;
    string8 name = names[node0];
    if (name.str[0] != 't')
      continue;
    vector *vec0 = *(vector **)kvi.value_ptr;
    u32 *neigh0 = (u32 *)vec0->data;
    for (u32 i = 0; i < vec0->size; ++i) {
      u32 node1 = neigh0[i];
      vector *vec1 = HM_GET(vector *, adj, &node1);
      u32 *neigh1 = (u32 *)vec1->data;
      for (u32 j = 0; j < vec1->size; ++j) {
        u32 node2 = neigh1[j];
        vector *vec2 = HM_GET(vector *, adj, &node2);
        if (vector_contains(vec2, &node0, u32_eql_fun)) {
          u32 nodes[3] = {node0, node1, node2};
          qsort(nodes, 3, sizeof(u32), cmp_fn);
          triplet tri = (triplet){.a = nodes[0], .b = nodes[1], .c = nodes[2]};
          hm_put(tris, &tri, &dummy);
        }
      }
    }
  }
  u64 n_tri = tris->size;
  hm_deinit(tris);
  return n_tri;
}

string8 find_max_clique(mem_arena *arena, hash_map *adj, vector *name_vec) {
  string8 *names = (string8 *)name_vec->data;
  u32 *nodes = malloc(sizeof(u32) * name_vec->size);
  for (u32 i = 0; i < name_vec->size; ++i) {
    nodes[i] = i;
  }
  u32 istart = name_vec->size;
  u32 max_size = 0;
  u32 prev = 0;
  u32 next = 1;
  while (next < name_vec->size) {
    for (u32 i = next; i < name_vec->size; ++i) {
      b8 add = 1;
      vector *vn = HM_GET(vector *, adj, &nodes[i]);
      for (u32 j = prev; j < next && add; ++j) {
        add = vector_contains(vn, &nodes[j], u32_eql_fun);
      }
      if (!add)
        continue;
      u32 tmp = nodes[i];
      nodes[i] = nodes[next];
      nodes[next] = tmp;
      next++;
    }
    u32 clique_size = next - prev;
    if (clique_size > max_size) {
      max_size = clique_size;
      istart = prev;
    }
    prev = next;
    next = prev + 1;
  }
  string8 *clique_names = malloc(sizeof(string8) * max_size);
  for (u32 i = 0; i < max_size; ++i) {
    clique_names[i] = names[nodes[istart + i]];
  }
  free(nodes);
  qsort(clique_names, max_size, sizeof(string8), str8_cmp);
  string8 sol = {0};
  str_join(arena, &sol, STR8_LIT(","), max_size, clique_names);
  free(clique_names);
  return sol;
}
