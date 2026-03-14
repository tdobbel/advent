#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_IMPLEMENTATION
#include "arena.h"

#define BUFSIZE 1024

typedef struct {
  u64 size;
  u8 *str;
} string8;

string8 clone_substring(mem_arena_tmp *tmp, string8 line, u64 start, u64 end) {
  u64 slice_size = end - start;
  u8 *slice = (u8 *)arena_push(tmp->arena, slice_size + 1);
  memcpy(slice, line.str + start, slice_size);
  slice[slice_size] = '\0';
  return (string8){.size = slice_size, .str = slice};
}

string8 substring(string8 s8, u64 start, u64 end) {
  return (string8){.size = (end - start), .str = s8.str + start};
}

typedef struct {
  string8 byr;
  string8 iyr;
  string8 eyr;
  string8 hgt;
  string8 hcl;
  string8 ecl;
  string8 pid;
  string8 cid;
} passport_t;

passport_t empty_passport() {
  return (passport_t){
      .byr = {0},
      .iyr = {0},
      .eyr = {0},
      .hgt = {0},
      .hcl = {0},
      .ecl = {0},
      .pid = {0},
      .cid = {0},
  };
}

b32 has_required_fields(passport_t *passport) {
  if (passport->byr.size == 0)
    return false;
  if (passport->iyr.size == 0)
    return false;
  if (passport->eyr.size == 0)
    return false;
  if (passport->hgt.size == 0)
    return false;
  if (passport->hcl.size == 0)
    return false;
  if (passport->ecl.size == 0)
    return false;
  if (passport->pid.size == 0)
    return false;
  return true;
}

void set_value(passport_t *passport, u8 *key, string8 value) {
  if (strcmp((char *)key, "byr") == 0)
    passport->byr = value;
  if (strcmp((char *)key, "iyr") == 0)
    passport->iyr = value;
  if (strcmp((char *)key, "eyr") == 0)
    passport->eyr = value;
  if (strcmp((char *)key, "hgt") == 0)
    passport->hgt = value;
  if (strcmp((char *)key, "hcl") == 0)
    passport->hcl = value;
  if (strcmp((char *)key, "ecl") == 0)
    passport->ecl = value;
  if (strcmp((char *)key, "pid") == 0)
    passport->pid = value;
  if (strcmp((char *)key, "cid") == 0)
    passport->cid = value;
}

b32 is_number(string8 entry) {
  for (u32 i = 0; i < entry.size; ++i) {
    if (entry.str[i] < '0' || entry.str[i] > '9')
      return false;
  }
  return true;
}

b32 valid_number_rng(string8 value, u32 lo, u32 hi) {
  if (!is_number(value))
    return false;
  u32 numValue = atoi((char *)value.str);
  return (numValue >= lo && numValue <= hi);
}

b32 valid_year(string8 value, u32 start_year, u32 end_year) {
  if (value.size != 4)
    return false;
  return valid_number_rng(value, start_year, end_year);
}

b32 valid_height(string8 value) {
  u64 iunits = value.size - 2;
  if (value.size < 4)
    return false;
  if (strcmp((char *)(value.str + iunits), "cm") == 0) {
    string8 num = substring(value, 0, iunits);
    return valid_number_rng(num, 150, 193);
  }
  if (strcmp((char *)(value.str + iunits), "in") == 0) {
    string8 num = substring(value, 0, iunits);
    return valid_number_rng(num, 59, 76);
  }
  return false;
}

b32 valid_hair_color(string8 value) {
  if (value.size != 7)
    return false;
  if (value.str[0] != '#')
    return false;
  for (u32 i = 1; i < value.size; ++i) {
    u8 c = value.str[i];
    if (c >= '0' && c <= '9')
      continue;
    if (c >= 'a' && c <= 'f')
      continue;
    return false;
  }
  return true;
}

b32 valid_eye_color(string8 value) {
  if (strcmp((char *)value.str, "amb") == 0)
    return true;
  if (strcmp((char *)value.str, "blu") == 0)
    return true;
  if (strcmp((char *)value.str, "brn") == 0)
    return true;
  if (strcmp((char *)value.str, "gry") == 0)
    return true;
  if (strcmp((char *)value.str, "grn") == 0)
    return true;
  if (strcmp((char *)value.str, "hzl") == 0)
    return true;
  if (strcmp((char *)value.str, "oth") == 0)
    return true;
  return false;
}

b32 valid_passport_id(string8 value) {
  if (value.size != 9)
    return false;
  for (u32 i = 0; i < value.size; ++i) {
    if (value.str[i] < '0' || value.str[i] > '9')
      return false;
  }
  return true;
}

b32 is_valid(passport_t *passport) {
  if (!has_required_fields(passport)) {
    return false;
  }
  if (!valid_year(passport->byr, 1920, 2002)) {
    printf("Invalid byr: %s\n", passport->byr.str);
    return false;
  }
  if (!valid_year(passport->iyr, 2010, 2020)) {
    printf("Invalid iyr: %s\n", passport->iyr.str);
    return false;
  }
  if (!valid_year(passport->eyr, 2020, 2030)) {
    printf("Invalid eyr: %s\n", passport->eyr.str);
    return false;
  }
  if (!valid_height(passport->hgt)) {
    printf("Invalid hgt: %s\n", passport->hgt.str);
    return false;
  }
  if (!valid_hair_color(passport->hcl)) {
    printf("Invalid hcl: %s\n", passport->hcl.str);
    return false;
  }
  if (!valid_eye_color(passport->ecl)) {
    printf("Invalid ecl: %s\n", passport->ecl.str);
    return false;
  }
  if (!valid_passport_id(passport->pid)) {
    printf("Invalid pid: %s\n", passport->pid.str);
    return false;
  }
  return true;
}

void parse_line(mem_arena_tmp *arena, passport_t *passport, string8 line) {
  u32 start = 0, end = 0;
  u8 key[4];
  while (end < line.size) {
    end = start;
    while (end < line.size && line.str[end] != ' ') {
      end++;
    }
    memcpy(key, line.str + start, 3);
    string8 value = clone_substring(arena, line, start + 4, end);
    set_value(passport, key, value);
    start = end + 1;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "Could not open file %s\n", argv[1]);
  }
  char buffer[BUFSIZE];
  passport_t passport = empty_passport();

  mem_arena *perm_area = arena_create(MiB(1));
  mem_arena_tmp temp_arena = arena_tmp_create(perm_area);

  u32 part1 = 0;
  u32 part2 = 0;

  while (fgets(buffer, BUFSIZE, fp)) {
    u32 n = strcspn(buffer, "\n");
    if (n == 0) {
      arena_tmp_end(temp_arena);
      temp_arena = arena_tmp_create(perm_area);
      if (has_required_fields(&passport)) {
        part1++;
        if (is_valid(&passport))
          part2++;
      }
      passport = empty_passport();
    }
    string8 line = {.size = n, .str = (u8 *)buffer};
    parse_line(&temp_arena, &passport, line);
  }

  if (has_required_fields(&passport)) {
    part1++;
    if (is_valid(&passport))
      part2++;
  }

  printf("Part 1: %u\n", part1);
  printf("Part 2: %u\n", part2);

  fclose(fp);
  arena_destroy(perm_area);

  return EXIT_SUCCESS;
}
