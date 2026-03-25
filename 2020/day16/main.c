#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_IMPLEMENTATION
#include "arena.h"

#define BUFSIZE 1024

typedef u8 b8;

typedef struct {
  u8 *str;
  u64 size;
} string8;

#define STR8_FMT "%.*s"
#define STR8_UNWRAP(s) (int)(s).size, (char *)(s).str

typedef struct {
  u8 indx, nposs;
} tuple;

int compare_tuple(const void *a, const void *b) {
  return ((tuple *)a)->nposs - ((tuple *)b)->nposs;
}

typedef struct {
  u32 rng1[2];
  u32 rng2[2];
} field_dfn;

inline b8 field_valid(field_dfn *fd, u32 value) {
  return (value >= fd->rng1[0] && value <= fd->rng1[1]) ||
         (value >= fd->rng2[0] && value <= fd->rng2[1]);
}

field_dfn parse_field(string8 line) {
  field_dfn fd;
  u32 num = 0;
  u32 i, r = 0;
  for (i = 0; line.str[i] != 'o'; ++i) {
    if (line.str[i] >= '0' && line.str[i] <= '9') {
      num = num * 10 + (u32)(line.str[i] - '0');
      continue;
    }
    fd.rng1[r++] = num;
    num = 0;
  }
  i += 3;
  r = 0;
  while (i <= line.size) {
    if (line.str[i] == '-' || i == line.size) {
      fd.rng2[r++] = num;
      num = 0;
    } else {
      num = num * 10 + (u32)(line.str[i] - '0');
    }
    i++;
  }
  return fd;
}

void parse_ticket(char *buffer, arena_vector *vec) {
  u32 n = strcspn(buffer, "\n");
  u32 num = 0;
  for (u32 i = 0; i <= n; ++i) {
    if (buffer[i] == ',' || i == n) {
      AVEC_PUSH(vec, u32, num);
      num = 0;
    } else {
      num = num * 10 + (u32)(buffer[i] - '0');
    }
  }
}

b8 valid_any(arena_vector *vec, u32 value) {
  field_dfn *fdef;
  for (u32 i = 0; i < vec->size; ++i) {
    fdef = (field_dfn *)vector_get(vec, i);
    if (field_valid(fdef, value))
      return 1;
  }
  return 0;
}

b8 solve_part2(tuple *tuples, i32 *field_ids, b8 *possible, u32 n_field,
               u32 i) {
  if (i == n_field)
    return 1;
  u32 indx = tuples[i].indx;
  for (u32 ifld = 0; ifld < n_field; ++ifld) {
    if (field_ids[ifld] >= 0)
      continue;
    if (!possible[indx * n_field + ifld])
      continue;
    field_ids[ifld] = indx;
    b8 solved = solve_part2(tuples, field_ids, possible, n_field, i + 1);
    if (solved)
      return 1;
    field_ids[ifld] = -1;
  }
  return 0;
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

  mem_arena *perm_arena = arena_create(MiB(1));
  char buffer[BUFSIZE];
  arena_vector *field_defs = AVEC_CREATE(perm_arena, field_dfn, 64);
  arena_vector *field_names = AVEC_CREATE(perm_arena, string8, 64);

  // Read ticket definitions
  u32 n_field = 0;
  while (fgets(buffer, BUFSIZE, fp)) {
    u32 n = strcspn(buffer, "\n");
    if (n == 0)
      break;
    u32 isep = strcspn(buffer, ":");
    u8 *str = PUSH_ARRAY(perm_arena, u8, isep);
    memcpy(str, buffer, isep);
    string8 fname = (string8){.str = str, .size = isep};
    AVEC_PUSH(field_names, string8, fname);
    isep += 2;
    string8 ranges = (string8){.str = (u8 *)buffer + isep, .size = n - isep};
    field_dfn fd = parse_field(ranges);
    AVEC_PUSH(field_defs, field_dfn, fd);
    n_field++;
  }

  // Read my own ticket
  arena_vector *my_ticket_ = AVEC_CREATE(perm_arena, u32, n_field);
  u32 dummy = 0;
  while (fgets(buffer, BUFSIZE, fp) && dummy < 1) {
    dummy++;
  }
  parse_ticket(buffer, my_ticket_);
  u32 *my_ticket = (u32 *)vector_flatten(my_ticket_);

  // Read all tickets
  dummy = 0;
  while (fgets(buffer, BUFSIZE, fp) && dummy < 1) {
    dummy++;
  }

  arena_vector *tickets = AVEC_CREATE(perm_arena, u32, 1024);
  u32 part1 = 0;
  u32 n_ticket = 0;
  while (fgets(buffer, BUFSIZE, fp)) {
    parse_ticket(buffer, tickets);
    b8 ok = 1;
    for (u32 i = 0; i < n_field; ++i) {
      u32 fvalue = *(u32 *)vector_get(tickets, n_field * n_ticket + i);
      if (!valid_any(field_defs, fvalue)) {
        ok = 0;
        part1 += fvalue;
      }
    }
    if (!ok) {
      tickets->size -= n_field;
    } else {
      n_ticket++;
    }
  }
  fclose(fp);

  u32 *test_tickets = (u32 *)vector_flatten(tickets);
  b8 *possible = (b8 *)PUSH_ARRAY(perm_arena, b8, n_ticket * n_field);
  tuple *nposs = (tuple *)PUSH_ARRAY(perm_arena, tuple, n_field);

  for (u32 indx = 0; indx < n_field; ++indx) {
    u32 total = 0;
    for (u32 ifld = 0; ifld < n_field; ++ifld) {
      field_dfn *fdef = vector_get(field_defs, ifld);
      b8 ok = 1;
      for (u32 itkt = 0; itkt < n_ticket; ++itkt) {
        u32 fvalue = test_tickets[itkt * n_field + indx];
        if (!field_valid(fdef, fvalue)) {
          ok = 0;
          break;
        }
      }
      possible[indx * n_field + ifld] = ok;
      total += ok;
      printf("%d ", ok);
    }
    nposs[indx] = (tuple){.indx = indx, .nposs = total};
    printf("-> %d\n", total);
  }

  qsort(nposs, n_field, sizeof(tuple), compare_tuple);

  i32 *field_ids = PUSH_ARRAY(perm_arena, i32, n_field);
  for (u32 i = 0; i < n_field; ++i) {
    field_ids[i] = -1;
  }

  b8 solved = solve_part2(nposs, field_ids, possible, n_field, 0);
  assert(solved);
  char word[10];
  u64 part2 = 1;
  word[9] = '\0';
  for (u32 i = 0; i < n_field; ++i) {
    u32 col = field_ids[i];
    string8 *fname = vector_get(field_names, i);
    if (fname->size < 9)
      continue;
    memcpy(word, fname->str, 9);
    if (strcmp(word, "departure") == 0) {
      printf(STR8_FMT ": %u\n", STR8_UNWRAP(*fname), my_ticket[col]);
      part2 *= (u64)my_ticket[col];
    }
  }

  arena_destroy(perm_arena);

  printf("Part1 : %u\n", part1);
  printf("Part2 : %lu\n", part2);

  return EXIT_SUCCESS;
}
