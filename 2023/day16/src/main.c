#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256

enum Direction { UP, RIGHT, DOWN, LEFT };

struct node_t {
  int x, y;
  enum Direction dir;
  struct node_t *next;
};

typedef struct {
  int capacity;
  struct node_t **arr;
} set_t;

typedef struct {
  struct node_t *head;
  struct node_t *tail;
} vector_t;

typedef struct {
  int nx, ny;
  char **grid;
} contraption_t;

contraption_t *read_contraption(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Could not open %s\n", filename);
    return NULL;
  }
  int ny = 0;
  int nx;
  char buffer[BUFFER_SIZE];
  int capacity = BUFFER_SIZE * BUFFER_SIZE;
  char *content = (char *)malloc(sizeof(char) * capacity);
  while (fgets(buffer, BUFFER_SIZE, file)) {
    nx = strcspn(buffer, "\n");
    for (int x = 0; x < nx; ++x) {
      int index = ny * nx + x;
      if (index >= capacity) {
        capacity *= 2;
        content = realloc(content, sizeof(char) * capacity);
      }
      content[ny * nx + x] = buffer[x];
    }
    ny++;
  }
  fclose(file);
  content = realloc(content, sizeof(char) * (nx * ny + 1));
  contraption_t *contraption = (contraption_t *)malloc(sizeof(contraption_t));
  contraption->nx = nx;
  contraption->ny = ny;
  contraption->grid = (char **)malloc(sizeof(char *) * ny);
  contraption->grid[0] = content;
  for (int y = 1; y < ny; ++y) {
    contraption->grid[y] = contraption->grid[y - 1] + nx;
  }
  return contraption;
}

int hash(set_t *set, int x, int y, enum Direction dir) {
  int index = 0, factor = 31;
  int num[3] = {x, y, dir};
  for (int i = 0; i < 3; ++i) {
    index = (31 * index + num[i]) % set->capacity;
  }
  return index;
}

set_t *set_create(int capacity) {
  set_t *set = (set_t *)malloc(sizeof(set_t));
  set->capacity = capacity;
  set->arr = (struct node_t **)malloc(sizeof(struct node_t *) * capacity);
  for (int i = 0; i < capacity; ++i) {
    set->arr[i] = NULL;
  }
  return set;
}

void reset(set_t *set) {
  for (int i = 0; i < set->capacity; ++i) {
    struct node_t *current = set->arr[i];
    while (current) {
      struct node_t *next = current->next;
      free(current);
      current = next;
    }
    set->arr[i] = NULL;
  }
}

int insert(set_t *set, int x, int y, enum Direction dir) {
  int index = hash(set, x, y, dir);
  struct node_t *node = set->arr[index];
  while (node) {
    if (node->x == x && node->y == y && node->dir == dir) {
      return 0;
    }
    node = node->next;
  }
  struct node_t *new_node = (struct node_t *)malloc(sizeof(struct node_t));
  new_node->x = x;
  new_node->y = y;
  new_node->dir = dir;
  new_node->next = set->arr[index];
  set->arr[index] = new_node;
  return 1;
}

int contraption_count_visited(contraption_t *puzzle, set_t *visited) {
  int *cntr = (int *)calloc(puzzle->ny * puzzle->nx, sizeof(int));
  for (int i = 0; i < visited->capacity; ++i) {
    struct node_t *node = visited->arr[i];
    while (node) {
      cntr[node->y * puzzle->nx + node->x] = 1;
      node = node->next;
    }
  }
  int n_visited = 0;
  for (int i = 0; i < puzzle->ny * puzzle->nx; ++i) {
    if (cntr[i])
      n_visited++;
  }
  free(cntr);
  return n_visited;
}

void set_free(set_t *set) {
  reset(set);
  free(set->arr);
  free(set);
}

void dir_to_dxdy(enum Direction dir, int *dx, int *dy) {
  *dx = 0;
  *dy = 0;
  switch (dir) {
  case UP:
    *dy = -1;
    break;
  case RIGHT:
    *dx = 1;
    break;
  case DOWN:
    *dy = 1;
    break;
  case LEFT:
    *dx = -1;
    break;
  }
}

void propagate_beam(contraption_t *puzzle, int x, int y, enum Direction dir,
                    set_t *visited) {
  // char *name[4] = {"UP", "RIGHT", "DOWN", "LEFT"};
  // printf("Current cell: (%d, %d) %s\n", y, x, name[dir]);
  if (!insert(visited, x, y, dir)) {
    return;
  }
  int next_dir[2] = {-1, -1};
  if (puzzle->grid[y][x] == '.') {
    next_dir[0] = dir;
  } else if (puzzle->grid[y][x] == '-') {
    if (dir == LEFT || dir == RIGHT) {
      next_dir[0] = dir;
    } else {
      next_dir[0] = (dir + 1) % 4;
      next_dir[1] = (dir + 3) % 4;
    }
  } else if (puzzle->grid[y][x] == '|') {
    if (dir == UP || dir == DOWN) {
      next_dir[0] = dir;
    } else {
      next_dir[0] = (dir + 1) % 4;
      next_dir[1] = (dir + 3) % 4;
    }
  } else if (puzzle->grid[y][x] == '\\') {
    if (dir == UP || dir == DOWN) {
      next_dir[0] = (dir + 3) % 4;
    } else {
      next_dir[0] = (dir + 1) % 4;
    }
  } else if (puzzle->grid[y][x] == '/') {
    if (dir == UP || dir == DOWN) {
      next_dir[0] = (dir + 1) % 4;
    } else {
      next_dir[0] = (dir + 3) % 4;
    }
  } else {
    fprintf(stderr, "This should not be happening\n");
    return;
  }
  int dx, dy;
  int x_new, y_new;
  for (int i = 0; i < 2; ++i) {
    if (next_dir[i] < 0)
      break;
    dir_to_dxdy(next_dir[i], &dx, &dy);
    x_new = x + dx;
    y_new = y + dy;
    if (x_new < 0 || x_new >= puzzle->nx || y_new < 0 || y_new >= puzzle->ny)
      continue;
    propagate_beam(puzzle, x_new, y_new, next_dir[i], visited);
  }
}

int n_visited_cells(contraption_t *puzzle, set_t *visited, int x, int y,
                    enum Direction dir) {
  reset(visited);
  propagate_beam(puzzle, x, y, dir, visited);
  return contraption_count_visited(puzzle, visited);
}

void contraption_free(contraption_t *contraption) {
  free(contraption->grid[0]);
  free(contraption->grid);
  free(contraption);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  contraption_t *puzzle = read_contraption(argv[1]);
  if (!puzzle) {
    return EXIT_FAILURE;
  }
  set_t *visited = set_create(2521);
  int part1 = n_visited_cells(puzzle, visited, 0, 0, RIGHT);
  printf("Part 1: %d\n", part1);
  int max_visited = 0;
  for (int x = 0; x < puzzle->nx; ++x) {
    int visited_top = n_visited_cells(puzzle, visited, x, 0, DOWN);
    if (visited_top > max_visited) {
      max_visited = visited_top;
    }
    int visited_bottom =
        n_visited_cells(puzzle, visited, x, puzzle->ny - 1, UP);
    if (visited_bottom > max_visited) {
      max_visited = visited_bottom;
    }
  }
  for (int y = 0; y < puzzle->ny; ++y) {
    int visited_left = n_visited_cells(puzzle, visited, 0, y, RIGHT);
    if (visited_left > max_visited) {
      max_visited = visited_left;
    }
    int visited_right =
        n_visited_cells(puzzle, visited, puzzle->nx - 1, y, LEFT);
    if (visited_right > max_visited) {
      max_visited = visited_right;
    }
  }

  printf("Part 2: %d\n", max_visited);

  contraption_free(puzzle);
  set_free(visited);

  return EXIT_SUCCESS;
}
