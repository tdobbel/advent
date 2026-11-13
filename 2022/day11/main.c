#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define ARENA_IMPLEMENTATION

#include "arena.h"

#define STRING_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION

#include "string8.h"
#include "vector.h"

typedef enum { Sum, Product } Operator;

typedef struct {
  u64 divider;
  u32 monkey_true, monkey_false;
} MonkeyTest;

typedef struct {
  Operator operator;
  u64 rhs;
} MonkeyOperation;

typedef struct {
  vector *worry_levels;
  MonkeyOperation operation;
  MonkeyTest test;
  u64 counter;
} Monkey;

u64 parse_line_number(string8 line);
MonkeyTest monkey_test_init(string8 *line);
u32 monkey_test_apply(MonkeyTest *self, u64 worry_level);
MonkeyOperation monkey_operation_init(string8 line);
u64 monkey_operation_apply(MonkeyOperation *self, u64 worry_level);
Monkey monkey_init(mem_arena *gpa, string8 *lines);
Monkey monkey_clone(const Monkey *self, mem_arena *gpa);
void next_round(Monkey *monkeys, u32 n_monkey, b8 divide_by_three);
u64 monkey_business(const Monkey *monkeys, u64 n_monkey);

int compare_fn(const void *a, const void *b) { return *(u64 *)b - *(u64 *)a; }

u64 GLOBAL_MOD = 1;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  mem_arena *gpa = arena_create(KiB(500));

  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);
  vector *line_vec = VEC_CREATE(string8);
  str_split(line_vec, str_trim(file), STR8_LIT("\n"));
  string8 *lines = VEC_CAST(line_vec, string8);

  vector *monkey_vec = VEC_ARENA_CREATE(gpa, Monkey);
  u32 ptr = 0;
  while (ptr < line_vec->size) {
    Monkey monkey = monkey_init(gpa, lines + ptr);
    vector_push(monkey_vec, &monkey);
    ptr += 7;
  }
  vector_deinit(line_vec);
  free(file.str);

  Monkey *monkeys = VEC_CAST(monkey_vec, Monkey);
  u64 n_monkey = monkey_vec->size;
  Monkey *monkeys_copy = ALLOC_ARRAY(gpa, Monkey, n_monkey);
  for (u64 i = 0; i < n_monkey; ++i) {
    monkeys_copy[i] = monkey_clone(&monkeys[i], gpa);
  }

  for (u32 i = 0; i < 20; ++i) {
    next_round(monkeys, n_monkey, 1);
  }
  printf("Part 1: %lu\n", monkey_business(monkeys, n_monkey));

  for (u32 i = 0; i < 10000; ++i) {
    next_round(monkeys_copy, n_monkey, 0);
  }
  printf("Part 2: %lu\n", monkey_business(monkeys_copy, n_monkey));

  arena_destroy(gpa);

  return EXIT_SUCCESS;
}

u64 parse_line_number(string8 line) {
  u64 start = line.size - 1;
  while (start > 0 && !isspace(line.str[start - 1])) {
    start--;
  }
  string8 last_word =
      (string8){.str = line.str + start, .size = line.size - start};
  return str_parse_unsigned(last_word);
}

MonkeyTest monkey_test_init(string8 *lines) {
  u64 divider = parse_line_number(lines[0]);
  GLOBAL_MOD *= divider;
  u32 monkey_true = parse_line_number(lines[1]);
  u32 monkey_false = parse_line_number(lines[2]);
  return (MonkeyTest){.divider = divider,
                      .monkey_true = monkey_true,
                      .monkey_false = monkey_false};
}

u32 monkey_test_apply(MonkeyTest *self, u64 worry_level) {
  return worry_level % self->divider == 0 ? self->monkey_true
                                          : self->monkey_false;
}

MonkeyOperation monkey_operation_init(string8 line) {
  u64 start = line.size - 1;
  while (!isspace(line.str[start - 1])) {
    start--;
  }
  string8 last_word =
      (string8){.str = line.str + start, .size = line.size - start};
  u64 rhs = 0;
  if (!str_equal(last_word, STR8_LIT("old"))) {
    rhs = str_parse_unsigned(last_word);
  }
  u8 c = line.str[start - 2];
  assert(c == '+' || c == '*');
  Operator op = c == '+' ? Sum : Product;
  return (MonkeyOperation){.operator= op, .rhs = rhs};
}

u64 monkey_operation_apply(MonkeyOperation *self, u64 worry_level) {
  u64 rhs = (self->rhs > 0) ? self->rhs : worry_level;
  switch (self->operator) {
  case Sum:
    return worry_level + rhs;
  case Product:
    return worry_level * rhs;
  }
}

Monkey monkey_init(mem_arena *gpa, string8 *lines) {
  MonkeyTest test = monkey_test_init(lines + 3);
  MonkeyOperation operation = monkey_operation_init(lines[2]);
  string8 splitted[2];
  str_split_once(splitted, lines[1], STR8_LIT(": "));
  vector *worry_levels = VEC_ARENA_CREATE(gpa, u64);
  vector *vec = VEC_CREATE(string8);
  str_split(vec, splitted[1], STR8_LIT(", "));
  string8 *words = VEC_CAST(vec, string8);
  for (u32 i = 0; i < vec->size; ++i) {
    u64 n = str_parse_unsigned(words[i]);
    vector_push(worry_levels, &n);
  }
  vector_deinit(vec);
  return (Monkey){
      .worry_levels = worry_levels,
      .operation = operation,
      .test = test,
      .counter = 0,
  };
}

Monkey monkey_clone(const Monkey *self, mem_arena *gpa) {
  vector *worry_copy = VEC_ARENA_CREATE(gpa, u64);
  u64 *worry_levels = VEC_CAST(self->worry_levels, u64);
  for (u32 i = 0; i < self->worry_levels->size; ++i) {
    vector_push(worry_copy, &worry_levels[i]);
  }
  return (Monkey){.worry_levels = worry_copy,
                  .operation = self->operation,
                  .test = self->test,
                  .counter = self->counter};
}

void next_round(Monkey *monkeys, u32 n_monkey, b8 divide_by_three) {
  for (u32 i = 0; i < n_monkey; ++i) {
    u64 worry;
    while (vector_pop(monkeys[i].worry_levels, &worry)) {
      u64 new_worry = monkey_operation_apply(&monkeys[i].operation, worry);
      if (divide_by_three) {
        new_worry /= 3;
      } else {
        new_worry %= GLOBAL_MOD;
      }
      monkeys[i].counter++;
      u32 index = monkey_test_apply(&monkeys[i].test, new_worry);
      vector_push(monkeys[index].worry_levels, &new_worry);
    }
  }
}

u64 monkey_business(const Monkey *monkeys, u64 n_monkey) {
  u64 *counters = (u64 *)malloc(n_monkey * sizeof(u64));
  for (u32 i = 0; i < n_monkey; ++i) {
    counters[i] = monkeys[i].counter;
  }
  qsort(counters, n_monkey, sizeof(u64), compare_fn);
  u64 ret = counters[0] * counters[1];
  free(counters);
  return ret;
}
