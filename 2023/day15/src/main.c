#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 25000

typedef struct {
  char data[12];
  int size;
} operation_t;

typedef struct {
  int capacity, size;
  operation_t *array;
} operation_list_t;

operation_list_t *operation_list_create() {
  operation_list_t *list = malloc(sizeof(operation_list_t));
  list->capacity = 1024;
  list->size = 0;
  list->array = malloc(list->capacity * sizeof(operation_t));
  return list;
}

struct node_t {
  char name[12];
  int flocal_length;
  struct node_t *next;
};

typedef struct {
  struct node_t **array;
} boxes_t;

void add_operation(operation_list_t *list, const char *data, int op_size) {
  if (list->size == list->capacity) {
    list->capacity *= 2;
    list->array = realloc(list->array, list->capacity * sizeof(operation_t));
  }
  operation_t *op = &list->array[list->size];
  strncpy(op->data, data, op_size);
  op->size = op_size;
  list->size++;
}

void operation_list_free(operation_list_t *list) {
  free(list->array);
  free(list);
}

boxes_t *boxes_create() {
  boxes_t *boxes = malloc(sizeof(boxes_t));
  boxes->array = malloc(256 * sizeof(struct node_t *));
  for (int i = 0; i < 256; ++i) {
    boxes->array[i] = NULL;
  }
  return boxes;
}

int hash(const char *name, int name_length) {
  int index = 0;
  for (int i = 0; i < name_length; ++i) {
    index += name[i];
    index *= 17;
    index %= 256;
  }
  return index;
}

void add_lens(boxes_t *boxes, const char *name, int name_length,
              int focal_length) {
  struct node_t *node = malloc(sizeof(struct node_t));
  strncpy(node->name, name, name_length);
  node->name[name_length] = '\0';
  node->flocal_length = focal_length;
  node->next = NULL;
  int index = hash(name, name_length);
  if (!boxes->array[index]) {
    boxes->array[index] = node;
  } else {
    struct node_t *current = boxes->array[index];
    struct node_t *last;
    while (current) {
      if (strcmp(current->name, node->name) == 0) {
        current->flocal_length = focal_length;
        free(node);
        return;
      }
      if (current->next == NULL) {
        last = current;
      }
      current = current->next;
    }
    last->next = node;
  }
}

void remove_lens(boxes_t *boxes, const char *name, int name_length) {
  int index = hash(name, name_length);
  struct node_t *current = boxes->array[index];
  struct node_t *prev = NULL;
  char name_copy[12];
  strncpy(name_copy, name, name_length);
  name_copy[name_length] = '\0';
  while (current) {
    if (strcmp(current->name, name_copy) == 0) {
      if (prev) {
        prev->next = current->next;
      } else {
        boxes->array[index] = current->next;
      }
      free(current);
      return;
    }
    prev = current;
    current = current->next;
  }
}

void boxes_free(boxes_t *boxes) {
  for (int i = 0; i < 256; ++i) {
    struct node_t *current = boxes->array[i];
    while (current) {
      struct node_t *temp = current;
      current = current->next;
      free(temp);
    }
  }
  free(boxes->array);
  free(boxes);
}

void print_boxes(boxes_t *boxes) {
  for (int i = 0; i < 256; ++i) {
    struct node_t *current = boxes->array[i];
    if (!current) {
      continue;
    }
    printf("Box %d: ", i);
    while (current) {
      printf("[%s %d] ", current->name, current->flocal_length);
      current = current->next;
    }
    printf("\n");
  }
}

void operation_list_apply(operation_list_t *operations, boxes_t *boxes) {
  for (int iop = 0; iop < operations->size; ++iop) {
    operation_t *op = &operations->array[iop];
    if (op->data[op->size - 1] == '-') {
      remove_lens(boxes, op->data, op->size - 1);
    } else {
      int ieq = strcspn(op->data, "=");
      int focal_length = op->data[ieq + 1] - '0';
      add_lens(boxes, op->data, ieq, focal_length);
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  char buffer[BUFFER_SIZE];
  FILE *file = fopen(argv[1], "r");
  if (!file) {
    fprintf(stderr, "Could not open file %s\n", argv[1]);
    return EXIT_FAILURE;
  }
  operation_list_t *operations = operation_list_create();
  fgets(buffer, BUFFER_SIZE, file);
  int n = strcspn(buffer, "\n");
  int part1 = 0;
  int hash = 0;
  char step[12];
  int j = 0;
  for (int i = 0; i < n; ++i) {
    if (buffer[i] == ',') {
      step[j] = '\0';
      add_operation(operations, step, j);
      part1 += hash;
      hash = 0;
      j = 0;
    } else {
      step[j++] = buffer[i];
      hash += buffer[i];
      hash *= 17;
      hash %= 256;
    }
  }
  part1 += hash;
  step[j] = '\0';
  add_operation(operations, step, j);
  fclose(file);

  printf("Part 1: %d\n", part1);

  boxes_t *boxes = boxes_create();
  int part2 = 0;
  operation_list_apply(operations, boxes);
  for (int ibox = 0; ibox < 256; ++ibox) {
    int islot = 1;
    struct node_t *current = boxes->array[ibox];
    while (current) {
      part2 += (ibox + 1) * islot * current->flocal_length;
      current = current->next;
      islot++;
    }
  }

  printf("Part 2: %d\n", part2);

  operation_list_free(operations);
  boxes_free(boxes);

  return EXIT_SUCCESS;
}
