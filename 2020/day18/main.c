#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#define BUFSIZE 1024

u64 parse(string8 s) {
  u64 res = 0;
  for (u64 i = 0; i < s.size; ++i) {
    res = res * 10 + (u64)(s.str[i] - '0');
  }
  return res;
}

u64 eval_no_parenthesis(string8 expr) {
  string8 sep = STR8_LIT(" ");
  vector *vec = split(expr, sep);
  string8 *data = (string8 *)vec->data;
  u64 total = parse(data[0]);
  for (u64 i = 1; i < vec->size - 1; i += 2) {
    u64 rhs = parse(data[i + 1]);
    if (data[i].str[0] == '+') {
      total += rhs;
    } else {
      total *= rhs;
    }
  }
  vector_free(vec);
  return total;
}

u64 solve_part1(string8 *input) {
  while (1) {
    b8 found = false;
    u64 ptr = 0;
    u64 left = 0, right;
    while (ptr < input->size) {
      if (input->str[ptr] == '(') {
        found = true;
        left = ptr;
      }
      if (input->str[ptr] == ')') {
        right = ptr;
        u64 size = right - left - 1;
        string8 substring =
            (string8){.str = input->str + left + 1, .size = right - left - 1};

        u64 subtotal = eval_no_parenthesis(substring);
        char num_str[64];
        sprintf(num_str, "%lu", subtotal);
        assert(strlen(num_str) <= size + 1);
        u64 delta = size + 2 - strlen(num_str);
        u64 i = 0;
        while (i < strlen(num_str)) {
          input->str[left + i] = num_str[i];
          i++;
        }
        ptr = left + i;
        u64 j = 0;
        while (j < input->size - right) {
          input->str[ptr + j] = input->str[right + j + 1];
          j++;
        }
        input->size -= delta;
        break;
      }
      ptr++;
    }
    if (!found)
      break;
  }
  return eval_no_parenthesis(*input);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "Could not open file %s\n", argv[1]);
    return EXIT_FAILURE;
  }
  char buffer[BUFSIZE];
  u64 part1 = 0;
  while (fgets(buffer, BUFSIZE, fp)) {
    u64 n = strcspn(buffer, "\n");
    string8 line = {(u8 *)buffer, n};
    part1 += solve_part1(&line);
  }
  fclose(fp);

  printf("Part 1: %lu\n", part1);

  return EXIT_SUCCESS;
}
