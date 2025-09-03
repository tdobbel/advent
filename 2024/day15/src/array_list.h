#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

enum DataType { DT_INT, DT_CHAR };

typedef struct {
  enum DataType type;
  union {
    int int_value;
    char c_value;
  } data;
} ArrayItem;

typedef struct {
  enum DataType type;
  int size, capacity;
  ArrayItem *items;
} ArrayList;

ArrayList *createArrayList(enum DataType type, int capacity);
void append(ArrayList *self, ArrayItem item);
void appendInt(ArrayList *self, int value);
void appendChar(ArrayList *self, char value);
void freeArrayList(ArrayList *self);
int getInt(ArrayList *self, int index);
char getChar(ArrayList *self, int index);
int popInt(ArrayList *self);
int containsInt(ArrayList *self, int value);

#endif // ARRAY_LIST_H
