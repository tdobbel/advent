#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 4096
#define MIN(a, b) (((a) <= (b)) ? (a) : (b))
#define ABS_DIFF(a, b) (((b) >= (a)) ? (b) - (a) : (a) - (b))

typedef uint32_t u32;
typedef int32_t i32;

u32 find_minimum(i32 *xs, u32 size, u32 (*f)(i32 *, u32, i32),
                 i32 (*df)(i32 *, u32, i32)) {
  i32 a = INT32_MAX;
  i32 b = INT32_MIN;
  u32 i;
  for (i = 0; i < size; ++i) {
    if (xs[i] < a)
      a = xs[i];
    if (xs[i] > b) {
      b = xs[i];
    }
  }
  while (b - a > 1) {
    i32 x = (a + b) / 2;
    i32 slope = (*df)(xs, size, x);
    if (slope == 0)
      return (*f)(xs, size, x);
    else if (slope < 0)
      a = x;
    else
      b = x;
  }
  return MIN((*f)(xs, size, a), (*f)(xs, size, b));
}

u32 cost1(i32 *xs, u32 size, i32 x) {
  u32 sum = 0;
  for (u32 i = 0; i < size; ++i) {
    sum += ABS_DIFF(xs[i], x);
  }
  return sum;
}

i32 slope1(i32 *xs, u32 size, i32 x) {
  return cost1(xs, size, x + 1) - cost1(xs, size, x);
}

u32 distance(i32 a, i32 b) {
  u32 n = ABS_DIFF(a, b);
  u32 d = 0;
  for (u32 i = 1; i <= n; ++i) {
    d += i;
  }
  return d;
}

u32 cost2(i32 *xs, u32 size, i32 x) {
  u32 sum = 0;
  for (u32 i = 0; i < size; ++i) {
    sum += distance(xs[i], x);
  }
  return sum;
}

i32 slope2(i32 *xs, u32 size, i32 x) {
  return cost2(xs, size, x + 1) - cost2(xs, size, x);
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(argv[1], "r");
  if (!fp) {
    fprintf(stderr, "Could not open file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  char buffer[BUFSIZE];
  fgets(buffer, BUFSIZE, fp);
  u32 line_size = strcspn(buffer, "\n");
  u32 size = 0, capacity = 64;
  i32 value = 0;
  i32 *xs = malloc(capacity * sizeof(i32));
  for (u32 i = 0; i <= line_size; ++i) {
    if (buffer[i] == '\n' || buffer[i] == ',') {
      if (size == capacity) {
        capacity *= 2;
        xs = realloc(xs, capacity * sizeof(i32));
      }
      xs[size++] = value;
      value = 0;
      continue;
    }
    value = value * 10 + buffer[i] - '0';
  }
  fclose(fp);
  xs = realloc(xs, size * sizeof(i32));

  u32 part1 = find_minimum(xs, size, cost1, slope1);
  printf("Part 1: %d\n", part1);

  u32 part2 = find_minimum(xs, size, cost2, slope2);
  printf("Part 2: %d\n", part2);


  free(xs);

  return EXIT_SUCCESS;
}
