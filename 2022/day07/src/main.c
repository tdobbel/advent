#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 250
#define N_CHILDREN 30
#define TOTAL_SPACE 70000000
#define REQUIRED_SPACE 30000000

struct Directory {
  char name[30];
  int fsize;
  struct Directory *parent;
  struct Directory *firstChild;
  struct Directory *nextSibling;
};

struct Directory *createDirectory(char *name, struct Directory *parent) {
  struct Directory *dir = (struct Directory *)malloc(sizeof(struct Directory));
  strcpy(dir->name, name);
  dir->parent = parent;
  dir->fsize = 0;
  dir->firstChild = NULL;
  dir->nextSibling = NULL;
  return dir;
}

void freeDirectories(struct Directory *root) {
  struct Directory *next, *tmp;
  next = root->firstChild;
  free(root);
  while (next != NULL) {
    next->parent = NULL;
    tmp = next->nextSibling;
    next->nextSibling = NULL;
    freeDirectories(next);
    next = tmp;
  }
}

void addChildren(struct Directory *current, char *name) {
  struct Directory *child = createDirectory(name, current);
  if (current->firstChild == NULL) {
    current->firstChild = child;
    return;
  }
  struct Directory *sibling = current->firstChild;
  while (sibling->nextSibling != NULL) {
    sibling = sibling->nextSibling;
  }
  sibling->nextSibling = child;
}

struct Directory *getChildren(struct Directory *current, char *name) {
  struct Directory *dir = current->firstChild;
  while (dir != NULL) {
    if (strcmp(dir->name, name) == 0) {
      return dir;
    }
    dir = dir->nextSibling;
  }
  return NULL;
}

int getDirectorySize(struct Directory *dir) {
  int size = dir->fsize;
  struct Directory *child = dir->firstChild;
  while (child != NULL) {
    size += getDirectorySize(child);
    child = child->nextSibling;
  }
  return size;
}

int summedSizeBelow(struct Directory *root, int maxSize) {
  int total = 0;
  int size = getDirectorySize(root);
  if (size <= maxSize) {
    total += size;
  }
  struct Directory *child = root->firstChild;
  while (child != NULL) {
    total += summedSizeBelow(child, maxSize);
    child = child->nextSibling;
  }
  return total;
}

void minimumFreeSpaced(struct Directory *root, int target, int *best) {
  int size = getDirectorySize(root);
  if (size < target)
    return;
  if (size < *best) {
    *best = size;
  }
  struct Directory *child = root->firstChild;
  while (child != NULL) {
    minimumFreeSpaced(child, target, best);
    child = child->nextSibling;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  char buffer[BUFFER_SIZE];

  struct Directory *root = createDirectory("/", NULL);
  struct Directory *current = root;
  int first = 1;
  while (fgets(buffer, BUFFER_SIZE, file)) {
    if (first) {
      first = 0;
      continue;
    }
    int n = strcspn(buffer, "\n");
    buffer[n] = '\0';
    if (buffer[0] == '$') {
      if (strncmp(buffer + 2, "cd", 2) == 0) {
        char *name = buffer + 5;
        if (strncmp(name, "..", 2) == 0) {
          current = current->parent;
        } else {
          current = getChildren(current, buffer + 5);
        }
      } else if (strncmp(buffer + 2, "ls", 2) == 0) {
        continue;
      }
    } else if (strncmp(buffer, "dir", 3) == 0) {
      addChildren(current, buffer + 4);
    } else {
      int size = 0;
      int sep = 0;
      for (sep = 0; sep < n && buffer[sep] != ' '; ++sep) {
        size = 10 * size + buffer[sep] - '0';
      }
      current->fsize += size;
    }
  }

  printf("Part 1: %d\n", summedSizeBelow(root, 100000));

  int availableSpace = TOTAL_SPACE - getDirectorySize(root);
  int target = REQUIRED_SPACE - availableSpace;
  int part2 = REQUIRED_SPACE;
  minimumFreeSpaced(root, target, &part2);
  printf("Part 2: %d\n", part2);

  freeDirectories(root);

  fclose(file);
  return 0;
}
