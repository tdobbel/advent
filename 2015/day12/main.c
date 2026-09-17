#include <stdio.h>
#include <stdlib.h>

#define JSON_IMPLEMENTATION
#include "json.h"

#include "hash_map.h"
#include "string8.h"
#include "vector.h"

void count_total(JsonValue *js, i64 *total, b8 ignore_red) {
  i64 inum;
  f64 fnum;
  string8 s;
  vector *vec = NULL;
  hash_map *hm = NULL;
  switch (js->type) {
  case Null:
  case Bool:
    fprintf(stderr, "Unexpected json data type");
    exit(1);
  case String:
    break;
  case Int:
    inum = *(i64 *)js->value;
    *total += inum;
    break;
  case Float:
    fnum = *(f64 *)js->value;
    *total += (i64)fnum;
    break;
  case Array:
    vec = (vector *)js->value;
    JsonValue **array = (JsonValue **)vec->data;
    for (u64 i = 0; i < vec->size; ++i) {
      count_total(array[i], total, ignore_red);
    }
    break;
  case Object:
    hm = (hash_map *)js->value;
    if (ignore_red) {
      kv_iterator kvi = hm_iterator(hm);
      while (get_next(&kvi)) {
        JsonValue *value = *(JsonValue **)kvi.value_ptr;
        if (value->type != String)
          continue;
        s = *(string8 *)value->value;
        if (str_equal(s, STR8_LIT("red")))
          return;
      }
    }
    kv_iterator kvi = hm_iterator(hm);
    while (get_next(&kvi)) {
      JsonValue *value = *(JsonValue **)kvi.value_ptr;
      count_total(value, total, ignore_red);
    }
    break;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file");
    return EXIT_FAILURE;
  }

  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);
  string8 s = str_remove_suffix(file, STR8_LIT("\n"));
  JsonValue *js = json_parse(s);

  i64 part1 = 0, part2 = 0;
  count_total(js, &part1, 0);
  count_total(js, &part2, 1);

  printf("Part 1:%ld\nPart 2: %ld\n", part1, part2);

  json_free(js);
  free(file.str);

  return EXIT_SUCCESS;
}
