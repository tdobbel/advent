#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION

#include "string8.h"
#include "vector.h"

typedef struct dir_struct Directory;

struct dir_struct {
  string8 name;
  u64 file_size, total_size;
  Directory *parent;
  vector *children;
};

Directory *directory_init(string8 name, Directory *parent);
void directory_add_child(Directory *parent, string8 name);
Directory *directory_get_child(Directory *parent, string8 child_name);
void directory_free(Directory *self);
void directory_compute_total_size(Directory *self);
void total_size_below_limit(u64 *total, const Directory *dir, u64 upper);
void minimum_free_above_target(u64 *min_free, const Directory *dir, u64 target);

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

  Directory *root = directory_init(STR8_LIT("\n"), NULL);
  Directory *current = root;
  string8 splitted[2] = {0};

  string8 space = STR8_LIT(" ");
  string8 ls = STR8_LIT("ls");
  string8 cd = STR8_LIT("cd ");
  string8 dotdot = STR8_LIT("..");

  for (u32 i = 1; i < line_vec->size; ++i) {
    if (lines[i].str[0] == '$') {
      // parse command
      string8 cmd =
          (string8){.str = lines[i].str + 2, .size = lines[i].size - 2};
      if (str_starts_with(cmd, ls))
        continue;
      assert(str_starts_with(cmd, cd));
      str_split_once(splitted, cmd, space);
      if (str_equal(splitted[1], dotdot)) {
        current = current->parent;
      } else {
        Directory *child = directory_get_child(current, splitted[1]);
        assert(!(child == NULL));
        current = child;
      }
    } else {
      str_split_once(splitted, lines[i], space);
      if (splitted[0].str[0] == 'd') {
        directory_add_child(current, splitted[1]);
      } else {
        u64 file_size = str_parse_unsigned(splitted[0]);
        current->file_size += file_size;
      }
    }
  }

  directory_compute_total_size(root);

  u64 part1 = 0;
  total_size_below_limit(&part1, root, 100000);
  printf("Part 1: %lu\n", part1);

  u64 total_disk_space = 70000000;
  u64 required_space = 30000000;
  u64 free_space = total_disk_space - root->total_size;
  u64 target_size = required_space - free_space;

  u64 part2 = total_disk_space;
  minimum_free_above_target(&part2, root, target_size);
  printf("Part 2: %lu\n", part2);

  vector_deinit(line_vec);

  directory_free(root);
  free(file.str);
}

Directory *directory_init(string8 name, Directory *parent) {
  Directory *dir = (Directory *)malloc(sizeof(Directory));
  dir->total_size = 0;
  dir->file_size = 0;
  dir->name = name;
  dir->parent = parent;
  dir->children = VEC_CREATE(Directory *);
  return dir;
}

void directory_add_child(Directory *parent, string8 name) {
  Directory *child = directory_init(name, parent);
  vector_push(parent->children, &child);
}

Directory *directory_get_child(Directory *parent, string8 child_name) {
  Directory **children = VEC_CAST(parent->children, Directory *);
  for (u32 i = 0; i < parent->children->size; ++i) {
    if (str_equal(child_name, children[i]->name))
      return children[i];
  }
  return NULL;
}

void directory_free(Directory *self) {
  Directory **children = VEC_CAST(self->children, Directory *);
  for (u32 i = 0; i < self->children->size; ++i) {
    directory_free(children[i]);
  }
  vector_deinit(self->children);
  free(self);
}

void directory_compute_total_size(Directory *self) {
  self->total_size = self->file_size;
  Directory **children = VEC_CAST(self->children, Directory *);
  for (u32 i = 0; i < self->children->size; ++i) {
    directory_compute_total_size(children[i]);
    self->total_size += children[i]->total_size;
  }
}

void total_size_below_limit(u64 *total, const Directory *dir, u64 upper) {
  if (dir->total_size < upper)
    *total += dir->total_size;
  Directory **children = VEC_CAST(dir->children, Directory *);
  for (u32 i = 0; i < dir->children->size; ++i) {
    total_size_below_limit(total, children[i], upper);
  }
}

void minimum_free_above_target(u64 *min_free, const Directory *dir,
                               u64 target) {
  if (dir->total_size < target)
    return;
  *min_free = MIN(*min_free, dir->total_size);
  Directory **children = VEC_CAST(dir->children, Directory *);
  for (u32 i = 0; i < dir->children->size; ++i) {
    minimum_free_above_target(min_free, children[i], target);
  }
}
