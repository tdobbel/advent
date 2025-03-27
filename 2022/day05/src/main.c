#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define N_CRATE 9

typedef struct {
  int size;
  char crates[100];
} Stack;

void addCrate(Stack *stack, char crate) {
  stack->crates[stack->size++] = crate;
}

void reverseCrates(Stack *stack) {
  char tmp;
  int j;
  for (int i = 0; i < stack->size / 2; ++i) {
    j = stack->size - i - 1;
    tmp = stack->crates[i];
    stack->crates[i] = stack->crates[j];
    stack->crates[j] = tmp;
  }
}

void moveCrates2(int n, Stack *sfrom, Stack *sto) {
  char crate;
  int j = sfrom->size - n;
  for (int i = 0; i < n; ++i) {
    addCrate(sto, sfrom->crates[j + i]);
  }
  sfrom->size -= n;
}

void moveCrates(int n, Stack *sfrom, Stack *sto) {
  char crate;
  for (int i = 0; i < n; ++i) {
    crate = sfrom->crates[sfrom->size - 1];
    addCrate(sto, crate);
    sfrom->size--;
  }
}

void printTops(Stack *stacks) {
  Stack *stack;
  for (int i = 0; i < N_CRATE; ++i) {
    stack = &stacks[i];
    printf("%c", stack->crates[stack->size - 1]);
  }
  printf("\n");
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return 1;
  }

  char buffer[BUFFER_SIZE];
  FILE *file = fopen(argv[1], "r");
  Stack *stacks1 = (Stack *)malloc(N_CRATE * sizeof(Stack));
  Stack *stacks2 = (Stack *)malloc(N_CRATE * sizeof(Stack));
  int i;
  for (i = 0; i < N_CRATE; ++i) {
    Stack *stack;
    stack = &stacks1[i];
    stack->size = 0;
    stack = &stacks2[i];
    stack->size = 0;
  }
  while (fgets(buffer, BUFFER_SIZE, file)) {
    char *p = strstr(buffer, "[");
    if (!p) {
      break;
    }
    int n = strcspn(buffer, "\n");
    i = 0;
    while (i < n) {
      if (buffer[i] == '[') {
        addCrate(&stacks1[i / 4], buffer[i + 1]);
      }
      i += 4;
    }
  }
  for (i = 0; i < N_CRATE; ++i) {
    Stack *stack = &stacks1[i];
    reverseCrates(stack);
    for (int j = 0; j < stack->size; ++j) {
      addCrate(&stacks2[i], stack->crates[j]);
    }
  }

  int nCrate, ifrom, ito;
  while (fgets(buffer, BUFFER_SIZE, file)) {
    int size = strcspn(buffer, "\n");
    if (size == 0) {
      continue;
    }
    sscanf(buffer, "move %d from %d to %d", &nCrate, &ifrom, &ito);
    moveCrates(nCrate, &stacks1[ifrom - 1], &stacks1[ito - 1]);
    moveCrates2(nCrate, &stacks2[ifrom - 1], &stacks2[ito - 1]);
  }
  fclose(file);

  printf("Part 1: ");
  printTops(stacks1);
  printf("Part 2: ");
  printTops(stacks2);

  free(stacks1);
  free(stacks2);
  return 0;
}
