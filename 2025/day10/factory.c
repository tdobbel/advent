#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 4096
#define ABS(x) (((x) >= 0) ? (x) : -(x))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef float f32;
typedef int32_t i32;
typedef u32 b32;

typedef struct {
  u32 capacity, length, elem_size;
  void *data;
} Vector;

typedef struct {
  u32 n_digit, diagram;
  Vector *buttons;
  Vector *requirements;
} Machine;

typedef struct {
  u32 shape[2];
  f32 **A;
  f32 *b, *x;
  i32 *xi;
  b32 *isfree;
  u16 *upper;
} System;

Vector *vector_create(u32 capacity, u32 elem_size);
void *vector_get(Vector *self, u32 index);
void *vector_append(Vector *self);
void vector_free(Vector *self);
void vector_empty(Vector *self) { self->length = 0; }

void machine_parse(Machine *machine, char *line);
void machine_display(Machine *self);
b32 machine_button_is_active(Machine *self, u32 req_index, u32 button_index);

void solve_part1(Machine *machine, u32 index, u32 word, u8 cost, u8 *min_cost);

void swapf(f32 *a, f32 *b);
void system_swap_rows(System *sys, u32 i, u32 j);

void system_build(System *sys, Machine *machine);
void system_display(System *sys);
void system_gaussian_elimination(System *sys);
void system_solve(System *sys);
u32 system_integer_solution(System *sys, Machine *machine);
void system_free(System *sys);
void system_print_solution(System *sys);

void solve_part2(Machine *machine, System *sys, u32 col, u32 *min_cost);

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

  char line[BUFSIZE];
  Machine machine = {.buttons = vector_create(16, sizeof(u32)),
                     .requirements = vector_create(16, sizeof(u16))};
  u32 part1 = 0, part2 = 0;
  System sys = {};
  while (fgets(line, BUFSIZE, fp)) {
    machine_parse(&machine, line);
    // machine_display(&machine);
    u8 sol1 = UINT8_MAX;
    solve_part1(&machine, 0, 0, 0, &sol1);
    part1 += sol1;

    system_build(&sys, &machine);
    system_gaussian_elimination(&sys);
    u32 sol2 = UINT32_MAX;
    solve_part2(&machine, &sys, 0, &sol2);
    part2 += sol2;

    system_free(&sys);
  }
  fclose(fp);
  printf("Part 1: %u\n", part1);
  printf("Part 2: %u\n", part2);

  vector_free(machine.buttons);
  vector_free(machine.requirements);
  return EXIT_SUCCESS;
}

Vector *vector_create(u32 capacity, u32 elem_size) {
  Vector *vec = malloc(sizeof(Vector));
  vec->capacity = capacity;
  vec->length = 0;
  vec->elem_size = elem_size;
  vec->data = malloc(elem_size * capacity);
  return vec;
}

void *vector_get(Vector *self, u32 index) {
  assert(index < self->length);
  return self->data + self->elem_size * index;
}

void *vector_append(Vector *self) {
  if (self->length == self->capacity) {
    self->capacity *= 2;
    self->data = realloc(self->data, self->capacity * self->elem_size);
  }
  return self->data + (self->length++) * self->elem_size;
}

void vector_free(Vector *self) {
  free(self->data);
  free(self);
}

void machine_parse(Machine *machine, char *line) {
  vector_empty(machine->buttons);
  vector_empty(machine->requirements);
  u32 n = strcspn(line, "]");
  u32 diagram = 0;
  u32 n_digit = 0;
  for (u32 i = 1; i < n; ++i) {
    n_digit++;
    diagram <<= 1;
    if (line[i] == '#')
      diagram += 1;
  }
  machine->n_digit = n_digit;
  machine->diagram = diagram;
  u32 stop = strcspn(line, "{") - 1;
  u32 button = 0;
  for (u32 i = n + 2; i < stop; ++i) {
    if (line[i] == '(' || line[i] == ' ' || line[i] == ',') {
      continue;
    }
    if (line[i] == ')') {
      u32 *appended = (u32 *)vector_append(machine->buttons);
      *appended = button;
      button = 0;
      continue;
    }
    u8 num = line[i] - '0';
    button |= 1 << (n_digit - 1 - num);
  }
  n = strcspn(line, "}");
  u16 num = 0;
  for (u32 i = stop + 2; i <= n; ++i) {
    if (line[i] == ',' || i == n) {
      u16 *ptr = (u16 *)vector_append(machine->requirements);
      *ptr = num;
      num = 0;
      continue;
    }
    num = num * 10 + (line[i] - '0');
  }
}

b32 machine_button_is_active(Machine *self, u32 req_index, u32 button_index) {
  u32 button = *(u32 *)vector_get(self->buttons, button_index);
  u32 test = 1 << (self->n_digit - req_index - 1);
  return button & test;
}

void machine_display(Machine *self) {
  printf("Buttons:");
  for (u32 ibut = 0; ibut < self->buttons->length; ++ibut) {
    printf("(");
    for (u32 ireq = 0; ireq < self->n_digit; ++ireq) {
      if (machine_button_is_active(self, ireq, ibut))
        printf("%u,", ireq);
    }
    printf(") ");
  }
  printf("\nRequirements: {");
  for (u32 i = 0; i < self->requirements->length; ++i) {
    u16 num = *(u16 *)vector_get(self->requirements, i);
    printf("%hu, ", num);
  }
  printf("}\n");
}

void solve_part1(Machine *machine, u32 index, u32 word, u8 cost, u8 *min_cost) {
  if (word == machine->diagram) {
    if (cost < *min_cost)
      *min_cost = cost;
    return;
  }
  if (cost >= *min_cost || index >= machine->buttons->length)
    return;
  u32 button = *(u32 *)vector_get(machine->buttons, index);
  solve_part1(machine, index + 1, word, cost,
              min_cost); // button is not pressed
  solve_part1(machine, index + 1, word ^ button, cost + 1,
              min_cost); // button is pressed;
}

void system_build(System *sys, Machine *machine) {
  sys->shape[1] = machine->buttons->length;
  sys->shape[0] = machine->n_digit;
  sys->b = malloc(sizeof(f32) * sys->shape[0]);
  sys->x = malloc(sizeof(f32) * sys->shape[1]);
  u16 *requirements = (u16 *)machine->requirements->data;
  u32 *buttons = (u32 *)machine->buttons->data;
  sys->upper = malloc(sizeof(u16) * sys->shape[1]);
  for (u32 i = 0; i < sys->shape[1]; ++i) {
    sys->upper[i] = UINT16_MAX;
  }
  f32 *a = malloc(sizeof(f32) * sys->shape[0] * sys->shape[1]);
  for (u32 i = 0; i < sys->shape[0]; ++i) {
    sys->b[i] = (f32)requirements[i];
    for (u32 j = 0; j < sys->shape[1]; ++j) {
      if (machine_button_is_active(machine, i, j)) {
        a[i * sys->shape[1] + j] = 1.0;
        sys->upper[j] = MIN(sys->upper[j], requirements[i]);
      } else {
        a[i * sys->shape[1] + j] = 0.0;
      }
    }
  }
  sys->A = malloc(sizeof(f32 *) * sys->shape[0]);
  sys->A[0] = a;
  for (u32 i = 1; i < sys->shape[0]; ++i) {
    sys->A[i] = sys->A[i - 1] + sys->shape[1];
  }
  sys->isfree = malloc(sizeof(b32) * sys->shape[1]);
  sys->xi = malloc(sizeof(i32) * sys->shape[1]);
}

void system_display(System *sys) {
  printf("A=\n");
  for (u32 i = 0; i < sys->shape[0]; ++i) {
    for (u32 j = 0; j < sys->shape[1]; ++j) {
      printf(" %1.2f ", sys->A[i][j]);
    }
    printf("\n");
  }
  printf("\nb=");
  for (u32 i = 0; i < sys->shape[0]; ++i) {
    printf(" %1.2f ", sys->b[i]);
  }
  printf("\n");
}

void system_print_solution(System *sys) {
  printf("x=");
  for (u32 i = 0; i < sys->shape[1]; ++i) {
    printf(" %1.2f ", sys->x[i]);
  }
  printf("\n");
  printf("xi=");
  for (u32 i = 0; i < sys->shape[1]; ++i) {
    printf(" %d ", sys->xi[i]);
  }
  printf("\n");
}

void system_free(System *sys) {
  free(sys->x);
  free(sys->xi);
  free(sys->b);
  free(sys->A[0]);
  free(sys->A);
  free(sys->isfree);
  free(sys->upper);
}

void swapf(f32 *a, f32 *b) {
  f32 tmp = *b;
  *b = *a;
  *a = tmp;
}

void system_swap_rows(System *sys, u32 i, u32 j) {
  if (i == j)
    return;
  for (u32 k = 0; k < sys->shape[1]; ++k) {
    swapf(&sys->A[i][k], &sys->A[j][k]);
  }
  swapf(&sys->b[i], &sys->b[j]);
}

void system_gaussian_elimination(System *sys) {
  u32 h = 0, k = 0;
  while (h < sys->shape[0] && k < sys->shape[1]) {
    u32 imax = h;
    for (u32 i = h + 1; i < sys->shape[0]; ++i) {
      if (ABS(sys->A[i][k]) > ABS(sys->A[imax][k]))
        imax = i;
    }
    if (ABS(sys->A[imax][k]) < 1e-6) {
      k++;
      continue;
    }
    system_swap_rows(sys, imax, h);
    for (u32 i = h + 1; i < sys->shape[0]; ++i) {
      f32 fac = sys->A[i][k] / sys->A[h][k];
      sys->A[i][k] = 0.0;
      sys->b[i] -= fac * sys->b[h];
      for (u32 j = k + 1; j < sys->shape[1]; ++j) {
        sys->A[i][j] -= fac * sys->A[h][j];
      }
    }
    h++;
    k++;
  }
  for (u32 i = 0; i < sys->shape[1]; ++i) {
    sys->isfree[i] = true;
  }
  u32 j = 0;
  for (u32 i = 0; i < sys->shape[0] && j < sys->shape[1]; ++i) {
    while (j < sys->shape[1] && ABS(sys->A[i][j]) < 1e-6) {
      j++;
    }
    if (j < sys->shape[1])
      sys->isfree[j] = false;
  }
}

void system_solve(System *sys) {
  u32 n = 0;
  for (u32 i = 0; i < sys->shape[1]; ++i) {
    if (!sys->isfree[i]) {
      n++;
    }
  }
  u32 j = sys->shape[1];
  for (u32 i = n; i > 0; --i) {
    j--;
    while (sys->isfree[j] && j > 0) {
      j--;
    }
    sys->x[j] = sys->b[i - 1];
    for (u32 k = sys->shape[1] - 1; k > j; --k) {
      if (sys->isfree[k])
        continue;
      sys->x[j] -= sys->A[i - 1][k] * sys->x[k];
    }
    sys->x[j] /= sys->A[i - 1][j];
  }
  for (u32 i = 0; i < sys->shape[1]; ++i) {
    sys->xi[i] = (i32)roundf(sys->x[i]);
  }
}

u32 system_integer_solution(System *sys, Machine *machine) {
  u16 *target = (u16 *)machine->requirements->data;
  for (u32 i = 0; i < sys->shape[1]; ++i) {
    if (sys->xi[i] < 0)
      return UINT32_MAX;
  }
  for (u32 i = 0; i < sys->shape[0]; ++i) {
    u16 num = 0;
    for (u32 j = 0; j < sys->shape[1]; ++j) {
      if (machine_button_is_active(machine, i, j)) {
        num += (u16)sys->xi[j];
      }
    }
    if (num != target[i])
      return UINT32_MAX;
  }
  u32 isol = 0;
  for (u32 i = 0; i < sys->shape[1]; ++i) {
    isol += (u32)sys->xi[i];
  }
  return isol;
}

void solve_part2(Machine *machine, System *sys, u32 col, u32 *min_cost) {
  u32 ix = col;
  while (ix < sys->shape[1] && !sys->isfree[ix]) {
    ix++;
  }
  if (ix == sys->shape[1]) {
    system_solve(sys);
    // system_print_solution(sys);
    u32 sol = system_integer_solution(sys, machine);
    if (sol < *min_cost) {
      *min_cost = sol;
    }
    return;
  }
  f32 *b = malloc(sizeof(f32) * sys->shape[0]);
  memcpy(b, sys->b, sizeof(f32) * sys->shape[0]);
  for (u16 x_value = 0; x_value <= sys->upper[ix]; ++x_value) {
    sys->x[ix] = (f32)x_value;
    for (u32 i = 0; i < sys->shape[0]; ++i) {
      sys->b[i] = b[i] - sys->A[i][ix] * sys->x[ix];
    }
    solve_part2(machine, sys, ix + 1, min_cost);
  }
  free(b);
}
