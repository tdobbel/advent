#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_IMPLEMENTATION
#include "vector.h"

typedef int32_t i32;
typedef uint8_t u8;
typedef u8 b8;

#define BUFSIZE 64

enum keyword { Nop, Acc, Jmp };

typedef struct {
  enum keyword command;
  i32 arg;
} instruct_t;

instruct_t parse_instruction(char *line) {
  enum keyword cmd;
  char kwd[4];
  memcpy(kwd, line, 3);
  kwd[3] = '\0';
  if (strcmp(kwd, "acc") == 0) {
    cmd = Acc;
  } else if (strcmp(kwd, "nop") == 0) {
    cmd = Nop;
  } else if (strcmp(kwd, "jmp") == 0) {
    cmd = Jmp;
  } else {
    assert(0);
  }
  char sign = line[4];
  i32 arg = atoi(line + 5);
  if (sign == '-') {
    arg *= -1;
  }
  return (instruct_t){.command = cmd, .arg = arg};
}

i32 run_program(vector *program) {
  b8 *touched = (b8 *)malloc(program->size);
  memset(touched, false, program->size);
  instruct_t *prog = (instruct_t *)program->data;
  i32 acc = 0;
  i32 i = 0;
  while (!touched[i]) {
    touched[i] = true;
    switch (prog[i].command) {
    case Acc:
      acc += prog[i].arg;
      i++;
      break;
    case Jmp:
      i += prog[i].arg;
      break;
    case Nop:
      i++;
      break;
    }
  }
  free(touched);
  return acc;
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
  vector *program = VEC_CREATE(instruct_t);
  while (fgets(buffer, BUFSIZE, fp)) {
    int n = strcspn(buffer, "\n");
    buffer[n] = '\0';
    instruct_t ins = parse_instruction(buffer);
    VEC_PUSH(program, instruct_t, ins);
  }
  fclose(fp);

  i32 part1 = run_program(program);
  printf("Part 1: %d\n", part1);

  vector_free(program);

  return EXIT_SUCCESS;
}
