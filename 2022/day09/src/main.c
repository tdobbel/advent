#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int size, capacity;
  int *positions;
} Record;

typedef struct {
  int size;
  int *knots;
  Record *tailPositions;
} Rope;

Record *createRecord() {
  Record *record = (Record *)malloc(sizeof(Record));
  record->capacity = 128;
  record->size = 0;
  record->positions = (int *)malloc(record->capacity * 2 * sizeof(int));
  return record;
}

void addPosition(Record *record, int x, int y) {
  int n = record->size;
  for (int i = 0; i < n; ++i) {
    if (record->positions[2 * i + 0] == x &&
        record->positions[2 * i + 1] == y) {
      return;
    }
  }
  if (n >= record->capacity) {
    record->capacity *= 2;
    record->positions =
        (int *)realloc(record->positions, record->capacity * 2 * sizeof(int));
  }
  record->positions[2 * n + 0] = x;
  record->positions[2 * n + 1] = y;
  record->size++;
}

void freeRecord(Record *record) {
  free(record->positions);
  free(record);
}

Rope *createRope(int n) {
  Rope *rope = (Rope *)malloc(sizeof(Rope));
  rope->size = n;
  rope->knots = (int *)calloc(sizeof(int), 2 * rope->size);
  rope->tailPositions = createRecord();
  addPosition(rope->tailPositions, 0, 0);
  return rope;
}

int distance(int x1, int y1, int x2, int y2) {
  int dx = abs(x1 - x2);
  int dy = abs(y1 - y2);
  return dx > dy ? dx : dy;
}

int signum(int x) {
  if (x > 0)
    return 1;
  if (x < 0)
    return -1;
  return 0;
}

void moveRope(Rope *rope, int dx, int dy) {
  rope->knots[0] += dx;
  rope->knots[1] += dy;
  int xprev = rope->knots[0];
  int yprev = rope->knots[1];
  int x, y, dx_, dy_;
  for (int i = 1; i < rope->size; ++i) {
    x = rope->knots[i * 2 + 0];
    y = rope->knots[i * 2 + 1];
    if (distance(xprev, yprev, x, y) > 1) {
      dx_ = signum(xprev - x);
      dy_ = signum(yprev - y);
      rope->knots[i * 2 + 0] += dx_;
      rope->knots[i * 2 + 1] += dy_;
      if (i == rope->size - 1) {
        addPosition(rope->tailPositions, x + dx_, y + dy_);
      }
    }
    xprev = rope->knots[i * 2 + 0];
    yprev = rope->knots[i * 2 + 1];
  }
}

void applyMotion(Rope *rope, char dir, int n) {
  int dx = 0, dy = 0;
  switch (dir) {
  case 'U':
    dy = -1;
    break;
  case 'D':
    dy = 1;
    break;
  case 'L':
    dx = -1;
    break;
  case 'R':
    dx = 1;
    break;
  }
  for (int i = 0; i < n; ++i) {
    moveRope(rope, dx, dy);
  }
}

void freeRope(Rope *rope) {
  free(rope->knots);
  freeRecord(rope->tailPositions);
  free(rope);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
  }

  Rope *rope2 = createRope(2);
  Rope *rope10 = createRope(10);

  FILE *file = fopen(argv[1], "r");
  char buffer[16];
  char dir;
  int n;
  while (fgets(buffer, 16, file)) {
    sscanf(buffer, "%c %d", &dir, &n);
    applyMotion(rope2, dir, n);
    applyMotion(rope10, dir, n);
  }
  fclose(file);

  printf("Part 1:%d\n", rope2->tailPositions->size);
  printf("Part 2:%d\n", rope10->tailPositions->size);

  freeRope(rope2);
  freeRope(rope10);

  return 0;
}
