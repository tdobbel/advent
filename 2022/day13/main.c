#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION

#include "string8.h"
#include "vector.h"

#define JSON_IMPLEMENTATION
#include "json.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
  u32 id;
  JsonValue *js;
} IndexedValue;

Order compare_packets(const JsonValue *left, const JsonValue *right);
int compare_fn(const void *a, const void *b) {
  JsonValue *left = ((IndexedValue *)a)->js;
  JsonValue *right = ((IndexedValue *)b)->js;
  switch (compare_packets(left, right)) {
  case lt:
    return -1;
  case gt:
    return 1;
  case eq:
    return 0;
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
  str_split(line_vec, str_trim(file), STR8_LIT("\n"));
  string8 *lines = VEC_CAST(line_vec, string8);

  u32 ptr = 0;
  u32 part1 = 0;

  vector *packet_vec = VEC_CREATE(IndexedValue);
  IndexedValue dummy = (IndexedValue){.id = 0, .js = NULL};

  while (ptr < line_vec->size) {
    JsonValue *left = json_parse(lines[ptr + 0]);
    JsonValue *right = json_parse(lines[ptr + 1]);

    if (compare_packets(left, right) == lt) {
      part1 += ptr / 3 + 1;
    }
    // add left
    dummy.js = left;
    vector_push(packet_vec, &dummy);
    dummy.id++;

    // add right
    dummy.js = right;
    vector_push(packet_vec, &dummy);
    dummy.id++;

    ptr += 3;
  }

  printf("Part 1: %u\n", part1);

  // add first divider
  JsonValue *div1 = json_parse(STR8_LIT("[[2]]"));
  u32 indx1 = dummy.id;
  dummy.js = div1;
  vector_push(packet_vec, &dummy);
  dummy.id++;

  // add second divider
  JsonValue *div2 = json_parse(STR8_LIT("[[6]]"));
  u32 indx2 = dummy.id;
  dummy.js = div2;
  vector_push(packet_vec, &dummy);
  dummy.id++;

  IndexedValue *values = VEC_CAST(packet_vec, IndexedValue);
  qsort(values, packet_vec->size, sizeof(IndexedValue), compare_fn);
  u32 part2 = 1;
  for (u32 i = 0; i < packet_vec->size; ++i) {
    if (values[i].id == indx1 || values[i].id == indx2)
      part2 *= (i + 1);
    json_free(values[i].js);
  }

  printf("Part 2: %u\n", part2);

  vector_deinit(packet_vec);
  vector_deinit(line_vec);
  free(file.str);

  return EXIT_SUCCESS;
}

Order compare_packets(const JsonValue *left, const JsonValue *right) {
  i64 left_num, right_num;
  vector *left_vec, *right_vec;

  if (left->type == Int && right->type == Int) {
    left_num = *(i64 *)left->value;
    right_num = *(i64 *)right->value;
    if (left_num < right_num)
      return lt;
    if (left_num > right_num)
      return gt;
    return eq;
  }
  if (left->type == Int) {
    left_vec = VEC_CREATE(JsonValue *);
    vector_push(left_vec, &left);
    JsonValue left_list = (JsonValue){.type = Array, .value = left_vec};
    Order ret = compare_packets(&left_list, right);
    vector_deinit(left_vec);
    return ret;
  }
  if (right->type == Int) {
    right_vec = VEC_CREATE(JsonValue *);
    vector_push(right_vec, &right);
    JsonValue right_list = (JsonValue){.type = Array, .value = right_vec};
    Order ret = compare_packets(left, &right_list);
    vector_deinit(right_vec);
    return ret;
  }
  left_vec = (vector *)left->value;
  right_vec = (vector *)right->value;

  JsonValue **left_values = VEC_CAST(left_vec, JsonValue *);
  JsonValue **right_values = VEC_CAST(right_vec, JsonValue *);

  u64 n = MAX(left_vec->size, right_vec->size);
  for (u32 i = 0; i < n; ++i) {
    if (i == left_vec->size)
      return lt;
    if (i == right_vec->size)
      return gt;
    Order order = compare_packets(left_values[i], right_values[i]);
    if (order != eq)
      return order;
  }
  return eq;
}
