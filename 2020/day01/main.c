#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 32
#define BASE_CAPACITY 128

typedef uint64_t u64;
typedef uint32_t u32;
typedef u32 b32;

typedef struct {
  u64 size, capacity;
  u64 *values;
} vector_t;

void vector_push(vector_t *vec, u64 num) {
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    vec->values = (u64 *)realloc(vec->values, sizeof(u64) * vec->capacity);
  }
  vec->values[vec->size++] = num;
}

b32 solve_sum(vector_t *vec, u32 pos, u64 *nums, u32 size, u32 iNum) {
  u64 sum = 0;
  for (u64 i = 0; i < iNum; ++i) {
    sum += nums[i];
  }
  if (sum > 2020)
    return false;
  if (iNum == size) {
    return sum == 2020;
  }
  u64 imax = vec->size - size + iNum + 1;
  for (u32 i = pos; i < imax; ++i) {
    nums[iNum] = vec->values[i];
    if (solve_sum(vec, i + 1, nums, size, iNum + 1)) {
      return true;
    }
  }
  return false;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "Could not open file '%s'", argv[1]);
    return EXIT_FAILURE;
  }

  u64 *values = (u64 *)malloc(sizeof(u64) * BASE_CAPACITY);
  vector_t vec = {.size = 0, .capacity = BASE_CAPACITY, .values = values};
  char buffer[BUFSIZE];

  while (fgets(buffer, BUFSIZE, fp)) {
    vector_push(&vec, atoi(buffer));
  }

  u64 nums[3];
  b32 solved = solve_sum(&vec, 0, nums, 2, 0);
  if (!solved) {
    fprintf(stderr, "Could not solve part1\n");
    return EXIT_FAILURE;
  }
  printf("Part 1: %lu\n", nums[0] * nums[1]);
  solved = solve_sum(&vec, 0, nums, 3, 0);
  if (!solved) {
    fprintf(stderr, "Could not solve part2\n");
    return EXIT_FAILURE;
  }
  printf("Part 2: %lu\n", nums[0] * nums[1] * nums[2]);

  fclose(fp);
  free(vec.values);

  return EXIT_SUCCESS;
}