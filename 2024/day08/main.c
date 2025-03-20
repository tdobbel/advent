#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define MAX_LINES_SIZE 256
#define MAX_ANTENNAS 64

typedef struct {
  int capacity, size;
  int *nodes;
} Positions;

typedef struct {
  int capacity, size;
  char *names;
  Positions *antennas;
} AntennaMap;

AntennaMap *createAntennaMap() {
  AntennaMap *map = (AntennaMap *)malloc(sizeof(AntennaMap));
  map->capacity = MAX_ANTENNAS;
  map->size = 0;
  map->names = (char *)malloc(sizeof(char) * MAX_ANTENNAS);
  map->antennas = (Positions *)malloc(sizeof(Positions) * MAX_ANTENNAS);
  return map;
}

void addAntenna(AntennaMap *map, char name, int node) {
  for (int i = 0; i < map->size; ++i) {
    if (map->names[i] == name) {
      Positions *pos = &map->antennas[i];
      pos->nodes[pos->size++] = node;
      return;
    }
  }
  map->names[map->size] = name;
  Positions *pos = &map->antennas[map->size++];
  pos->nodes = (int *)malloc(sizeof(int) * MAX_ANTENNAS);
  pos->nodes[0] = node;
  pos->size = 1;
  pos->capacity = MAX_ANTENNAS;
}

void freeAntennaMap(AntennaMap *map) {
  for (int i = 0; i < map->size; ++i) {
    free(map->antennas[i].nodes);
  }
  free(map->antennas);
  free(map->names);
  free(map);
}

int validPosition(int x, int y, int nrows, int ncols) {
  return x >= 0 && x < ncols && y >= 0 && y < nrows;
}

void findAntinodesPart1(AntennaMap *map, int index, int nrows, int ncols,
                        int *antinodes) {
  int y0, x0, y1, x1, dx, dy, x, y;
  Positions *pos = &map->antennas[index];
  for (int i = 0; i < pos->size - 1; ++i) {
    for (int j = i + 1; j < pos->size; ++j) {
      y0 = pos->nodes[i] / ncols;
      x0 = pos->nodes[i] % ncols;
      y1 = pos->nodes[j] / ncols;
      x1 = pos->nodes[j] % ncols;
      dy = y1 - y0;
      dx = x1 - x0;
      x = x1 + dx;
      y = y1 + dy;
      if (validPosition(x, y, nrows, ncols)) {
        antinodes[y * ncols + x] = 1;
      }
      x = x0 - dx;
      y = y0 - dy;
      if (validPosition(x, y, nrows, ncols)) {
        antinodes[y * ncols + x] = 1;
      }
    }
  }
}

void findAntinodesPart2(AntennaMap *map, int index, int nrows, int ncols,
                        int *antinodes) {
  int y0, x0, y1, x1, dx, dy, x, y;
  Positions *pos = &map->antennas[index];
  for (int i = 0; i < pos->size - 1; ++i) {
    for (int j = i + 1; j < pos->size; ++j) {
      y0 = pos->nodes[i] / ncols;
      x0 = pos->nodes[i] % ncols;
      y1 = pos->nodes[j] / ncols;
      x1 = pos->nodes[j] % ncols;
      dy = y1 - y0;
      dx = x1 - x0;
      x = x1;
      y = y1;
      while (validPosition(x, y, nrows, ncols)) {
        antinodes[y * ncols + x] = 1;
        x += dx;
        y += dy;
      }
      x = x0;
      y = y0;
      while (validPosition(x, y, nrows, ncols)) {
        antinodes[y * ncols + x] = 1;
        x -= dx;
        y -= dy;
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  int nrows = 0;
  int ncols;
  char line[MAX_LINES_SIZE];
  AntennaMap *map = createAntennaMap();
  while (fgets(line, MAX_LINES_SIZE, file)) {
    ncols = strcspn(line, "\n");
    for (int i = 0; i < ncols; ++i) {
      if (line[i] == '.') {
        continue;
      }
      addAntenna(map, line[i], nrows * ncols + i);
    }
    nrows++;
  }
  fclose(file);

  // Part 1
  int *antinodes = (int *)calloc(nrows * ncols, sizeof(int));
  int i;
  for (i = 0; i < map->size; ++i) {
    findAntinodesPart1(map, i, nrows, ncols, antinodes);
  }
  int part1 = 0;
  for (i = 0; i < nrows * ncols; ++i) {
    part1 += antinodes[i];
  }
  printf("Part 1: %d\n", part1);

  // Part 2
  memset(antinodes, 0, nrows * ncols * sizeof(int));
  for (i = 0; i < map->size; ++i) {
    findAntinodesPart2(map, i, nrows, ncols, antinodes);
  }
  int part2 = 0;
  for (i = 0; i < nrows * ncols; ++i) {
    part2 += antinodes[i];
  }
  printf("Part 2: %d\n", part2);

  free(antinodes);
  freeAntennaMap(map);
  return 0;
}
