#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_IMPLEMENTATION
#include "vector.h"

typedef uint8_t u8;
typedef u8 b8;

#define BUFSIZE 64
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

u64 parse_number(char *line) {
  u64 n = strcspn(line, "\n");
  u64 num = 0;
  for (u64 i = 0; i < n; ++i) {
    num = num * 10 + (u64)(line[i] - '0');
  }
  return num;
}

b8 isvalid(u64 *nums, u64 size, u64 num) {
  for (u64 i = 0; i < size - 1; ++i) {
    if (nums[i] > num)
      continue;
    for (u64 j = i + 1; j < size; ++j) {
      if (nums[i] + nums[j] == num)
        return 1;
    }
  }
  return 0;
}

b8 solve_part2(vector *vec, u64 *sol) {
  u64 *nums = (u64 *)vec->data;
  u64 target = nums[vec->size - 1];
  for (u64 i = 0; i < vec->size - 2; ++i) {
    u64 vmin = UINT64_MAX;
    u64 vmax = 0;
    u64 sum = 0;
    for (u64 j = i; j < vec->size - 1; ++j) {
      u64 num = nums[j];
      if (UINT64_MAX - sum < num) // make sure that we don't overflow...
        break;
      sum += num;
      vmin = MIN(vmin, num);
      vmax = MAX(vmax, num);
      if (sum == target) {
        *sol = vmin + vmax;
        return true;
      }
      if (sum > target)
        break;
    }
  }
  return false;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Input file and preamble length needed\n");
    return EXIT_FAILURE;
  }
  u64 size = atoi(argv[2]);
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "Could not open file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  char buffer[BUFSIZE];
  vector *vec = VEC_CREATE(u64);
  for (u64 i = 0; i < size; ++i) {
    fgets(buffer, BUFSIZE, fp);
    VEC_PUSH(vec, u64, parse_number(buffer));
  }

  u64 num;
  while (fgets(buffer, BUFSIZE, fp)) {
    num = parse_number(buffer);
    VEC_PUSH(vec, u64, num);
    if (!isvalid((u64 *)vec->data + vec->size - size - 1, size, num)) {
      break;
    }
  }
  fclose(fp);

  printf("Part 1: %lu\n", num);

  u64 part2;
  assert(solve_part2(vec, &part2));
  printf("Part 2: %lu\n", part2);

  vector_free(vec);

  return EXIT_SUCCESS;
}
