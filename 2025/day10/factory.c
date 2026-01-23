#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 4096

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef u8 b8;

typedef struct {
  u32 capacity, length, elem_size;
  void *data;
} Vector;

typedef struct {
  u32 n_digit, diagram;
  Vector *buttons;
  Vector *requirements;
} Machine;

Vector *vector_create(u32 capacity, u32 elem_size);
void *vector_get(Vector *self, u32 index);
void *vector_append(Vector *self);
void vector_free(Vector *self);
void vector_empty(Vector *self) { self->length = 0; }

void machine_parse(Machine *machine, char *line);
void machine_display(Machine *machine);

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
  while (fgets(line, BUFSIZE, fp)) {
    machine_parse(&machine, line);
    machine_display(&machine);
  }
  vector_free(machine.buttons);
  vector_free(machine.requirements);
  fclose(fp);
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
    if (line[i] == '#') {
      n_digit++;
      diagram <<= 1;
      if (line[i] == '#')
        diagram += 1;
    }
  }
  machine->n_digit = n_digit;
  machine->diagram = diagram;
  u32 stop = strcspn(line, "{") - 1;
  b8 parsing = 0;
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

void machine_display(Machine *self) {
  printf("Buttons:");
  for (u32 i = 0; i < self->buttons->length; ++i) {
    u32 button = *(u32*)vector_get(self->buttons, i);
    printf("(");
    for (u32 p = 0; p < self->n_digit; ++p) {
      u32 test = 1 << (self->n_digit - p - 1);
      if (button & test)
        printf("%u,", p);
    }
    printf(") ");
  }
  printf("\nRequirements: {");
  for (u32 i = 0; i < self->requirements->length; ++i) {
    u16 num = *(u16*)vector_get(self->requirements, i);
    printf("%hu, ", num);
  }
  printf("}\n");
}
