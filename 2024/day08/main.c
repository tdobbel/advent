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

int addAntinode(int *antinodes, int nx, int ny, int x, int y) {
  if (x < 0 || x >= nx || y < 0 || y >= ny) {
    return 0;
  }
  antinodes[y * nx + x] = 1;
  return 1;
}

void findAntinodesPart1(AntennaMap *map, int index, int ny, int nx,
                        int *antinodes) {
  int yA, xA, yB, xB, dx, dy;
  Positions *pos = &map->antennas[index];
  for (int i = 0; i < pos->size - 1; ++i) {
    yA = pos->nodes[i] / nx;
    xA = pos->nodes[i] % nx;
    for (int j = i + 1; j < pos->size; ++j) {
      yB = pos->nodes[j] / nx;
      xB = pos->nodes[j] % nx;
      dy = yB - yA;
      dx = xB - xA;
      addAntinode(antinodes, nx, ny, xB + dx, yB + dy);
      addAntinode(antinodes, nx, ny, xA + dx, yA + dy);
    }
  }
}

void findAntinodesPart2(AntennaMap *map, int index, int ny, int nx,
                        int *antinodes) {
  int yA, xA, yB, xB, dx, dy, x, y;
  Positions *pos = &map->antennas[index];
  for (int i = 0; i < pos->size - 1; ++i) {
    yA = pos->nodes[i] / nx;
    xA = pos->nodes[i] % nx;
    for (int j = i + 1; j < pos->size; ++j) {
      yB = pos->nodes[j] / nx;
      xB = pos->nodes[j] % nx;
      dy = yB - yA;
      dx = xB - xA;
      x = xB;
      y = yB;
      while (addAntinode(antinodes, nx, ny, x, y)) {
        x += dx;
        y += dy;
      }
      x = xA;
      y = yA;
      while (addAntinode(antinodes, nx, ny, x, y)) {
        antinodes[y * nx + x] = 1;
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
