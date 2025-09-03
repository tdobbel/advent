#include "array_list.h"
#include <assert.h>
#include <stdlib.h>

ArrayList *createArrayList(enum DataType type, int capacity) {
  ArrayList *self = (ArrayList *)malloc(sizeof(ArrayList));
  self->size = 0;
  self->capacity = capacity;
  self->items = (ArrayItem *)malloc(capacity * sizeof(ArrayItem));
  self->type = type;
  return self;
}

void freeArrayList(ArrayList *self) {
  free(self->items);
  free(self);
}

void append(ArrayList *self, ArrayItem item) {
  assert(self->type == item.type);
  if (self->size == self->capacity) {
    self->capacity *= 2;
    self->items =
        (ArrayItem *)realloc(self->items, self->capacity * sizeof(ArrayItem));
  }
  self->items[self->size++] = item;
}

void appendInt(ArrayList *self, int value) {
  assert(self->type == DT_INT);
  ArrayItem item;
  item.type = DT_INT;
  item.data.int_value = value;
  append(self, item);
}

void appendChar(ArrayList *self, char value) {
  assert(self->type == DT_CHAR);
  ArrayItem item;
  item.type = DT_CHAR;
  item.data.c_value = value;
  append(self, item);
}

char getChar(ArrayList *self, int index) {
  assert(self->type == DT_CHAR);
  assert(index >= 0 && index < self->size);
  return self->items[index].data.c_value;
}

int getInt(ArrayList *self, int index) {
  assert(self->type == DT_INT);
  assert(index >= 0 && index < self->size);
  return self->items[index].data.int_value;
}

int popInt(ArrayList *self) {
  assert(self->type == DT_INT);
  assert(self->size > 0);
  return self->items[--self->size].data.int_value;
}

int containsInt(ArrayList *self, int value) {
  assert(self->type == DT_INT);
  for (int i = 0; i < self->size; ++i) {
    if (self->items[i].data.int_value == value) {
      return 1;
    }
  }
  return 0;
}
