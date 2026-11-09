#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION

#include "string8.h"
#include "vector.h"

typedef struct {
  u64 a, b, c;
} Register;

u64 register_combo(const Register *r, u64 literal);
b8 execute_operation(u8 *output, Register *r, const u64 *program, u32 *index);
void execute_program(vector *output, Register *r, const u64 *program,
                     u64 sizep);
b8 solve_part2(u64 *sol, const u64 *program, u64 sizep, u64 prev, u64 index);
void print_output(u8 *items, u64 n);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);
  vector *line_vec = VEC_CREATE(string8);
  str_split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = VEC_CAST(line_vec, string8);

  string8 splitted[2] = {0};
  string8 delim = STR8_LIT(": ");
  u64 nums[3] = {0};
  for (u32 i = 0; i < 3; ++i) {
    str_split_once(splitted, lines[i], delim);
    nums[i] = str_parse_unsigned(splitted[1]);
  }
  Register r = (Register){.a = nums[0], .b = nums[1], .c = nums[2]};

  str_split_once(splitted, lines[4], delim);
  vector *num_vec = VEC_CREATE(string8);
  str_split(num_vec, splitted[1], STR8_LIT(","));
  string8 *num_str = VEC_CAST(num_vec, string8);

  vector *program_vec = VEC_CREATE(u64);
  for (u32 i = 0; i < num_vec->size; ++i) {
    u64 num = str_parse_unsigned(num_str[i]);
    vector_push(program_vec, &num);
  }

  vector_deinit(line_vec);
  vector_deinit(num_vec);

  vector *out1 = VEC_CREATE(u8);
  u64 *program = VEC_CAST(program_vec, u64);
  execute_program(out1, &r, program, program_vec->size);
  printf("Part 1: ");
  print_output(VEC_CAST(out1, u8), out1->size);

  vector_deinit(out1);

  u64 part2;
  if (!solve_part2(&part2, program, program_vec->size, 0, program_vec->size)) {
    fprintf(stderr, "Could not solve part 2\n");
    return EXIT_FAILURE;
  }
  printf("Part 2: %lu\n", part2);

  vector_deinit(program_vec);

  free(file.str);

  return EXIT_SUCCESS;
}

u64 register_combo(const Register *r, u64 literal) {
  if (literal < 4) {
    return literal;
  } else if (literal == 4) {
    return r->a;
  } else if (literal == 5) {
    return r->b;
  } else if (literal == 6) {
    return r->c;
  } else {
    fprintf(stderr, "Not supposed to be here");
    exit(1);
  }
}

b8 execute_operation(u8 *output, Register *r, const u64 *program, u32 *index) {
  u64 op = program[*index];
  u64 literal = program[*index + 1];
  u64 combo = register_combo(r, literal);
  switch (op) {
  case 0:
    r->a = r->a >> combo;
    *index += 2;
    return false;
  case 1:
    r->b ^= literal;
    *index += 2;
    return false;
  case 2:
    r->b = combo % 8;
    *index += 2;
    return false;
  case 3:
    r->a == 0 ? (*index += 2) : (*index = literal);
    return false;
  case 4:
    r->b ^= r->c;
    *index += 2;
    return false;
  case 5:
    *output = combo % 8;
    *index += 2;
    return true;
  case 6:
    r->b = r->a >> combo;
    *index += 2;
    return false;
  case 7:
    r->c = r->a >> combo;
    *index += 2;
    return false;
  default:
    fprintf(stderr, "Invalid opcode: %lu\n", op);
    exit(EXIT_FAILURE);
  }
}

void print_output(u8 *items, u64 n) {
  for (u32 k = 0; k < n; ++k) {
    printf("%hu", items[k]);
    if (k < n - 1)
      printf(",");
  }
  printf("\n");
}

void execute_program(vector *output, Register *r, const u64 *program,
                     u64 sizep) {
  u32 index = 0;
  while (index < sizep) {
    u8 out;
    if (execute_operation(&out, r, program, &index)) {
      vector_push(output, &out);
    }
  }
}

b8 solve_part2(u64 *sol, const u64 *program, u64 sizep, u64 prev, u64 index) {
  if (index == 0) {
    *sol = prev;
    return true;
  }
  vector *output = VEC_CREATE(u8);
  b8 ret = false;
  for (u32 i = 0; i < 8; ++i) {
    u64 a = prev << 3 | i;
    Register r = (Register){.a = a, .b = 0, .c = 0};
    output->size = 0;
    execute_program(output, &r, program, sizep - 2);
    u8 vout;
    vector_get_last(output, &vout);
    if (vout != (u8)program[index - 1])
      continue;
    if (solve_part2(sol, program, sizep, a, index - 1)) {
      ret = true;
      break;
    }
  }
  vector_deinit(output);
  return ret;
}
