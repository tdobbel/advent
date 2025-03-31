#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 512

typedef struct {
  int x[2], y[2];
} Fence;

typedef struct {
  int nx, ny;
  char **grid;
} Plots;

Plots *createPlots(int n_row, int n_col, char *pixels) {
  Plots *plots = (Plots *)malloc(sizeof(Plots));
  plots->ny = n_row;
  plots->nx = n_col;
  plots->grid = (char **)malloc(sizeof(char *) * n_row);
  plots->grid[0] = pixels;
  for (int i = 1; i < n_row; ++i) {
    plots->grid[i] = plots->grid[i - 1] + n_col;
  }
  return plots;
}

void freePlots(Plots *plots) {
  free(plots->grid[0]);
  free(plots->grid);
  free(plots);
}

void setFencePoints(Fence *fence, int x, int y, char side) {
  switch (side) {
  case 'N':
    fence->x[0] = x + 1;
    fence->y[0] = y;
    fence->x[1] = x;
    fence->y[1] = y;
    break;
  case 'W':
    fence->x[0] = x;
    fence->y[0] = y;
    fence->x[1] = x;
    fence->y[1] = y + 1;
    break;
  case 'S':
    fence->x[0] = x;
    fence->y[0] = y + 1;
    fence->x[1] = x + 1;
    fence->y[1] = y + 1;
    break;
  case 'E':
    fence->x[0] = x + 1;
    fence->y[0] = y + 1;
    fence->x[1] = x + 1;
    fence->y[1] = y;
    break;
  default:
    printf("Invalid side\n");
    exit(1);
  }
}

void computePrice(const Plots *plots, const char region, int x, int y,
                  int *visited, int *area, int *perimeter, Fence *fences) {
  visited[y * plots->nx + x] = 1;
  *area += 1;
  int shiftx[4] = {0, 1, 0, -1};
  int shifty[4] = {1, 0, -1, 0};
  char side[4] = {'S', 'E', 'N', 'W'};
  int x_, y_;
  for (int k = 0; k < 4; ++k) {
    x_ = x + shiftx[k];
    y_ = y + shifty[k];
    if (x_ < 0 || x_ >= plots->nx || y_ < 0 || y_ >= plots->ny ||
        plots->grid[y_][x_] != region) {
      setFencePoints(fences + *perimeter, x, y, side[k]);
      *perimeter += 1;
      continue;
    }
    if (!visited[y_ * plots->nx + x_])
      computePrice(plots, region, x_, y_, visited, area, perimeter, fences);
  }
}

int countCorners(Fence *fences, int perimeter) {
  int corners = 0;
  int remaining = perimeter;
  int u[2], v[2];
  int *touched = (int *)calloc(sizeof(int), perimeter);
  int icurr, found, cntr, first;
  while (remaining > 0) {
    icurr = 0;
    Fence *curr = &fences[icurr];
    while (!touched[0]) {
      found = 0;
      for (int i = 0; i < remaining; ++i) {
        if (i == icurr || touched[i])
          continue;
        Fence *next = &fences[i];
        if (next->x[0] == curr->x[1] && next->y[0] == curr->y[1]) {
          u[0] = curr->x[1] - curr->x[0];
          u[1] = curr->y[1] - curr->y[0];
          v[0] = next->x[1] - next->x[0];
          v[1] = next->y[1] - next->y[0];
          if (u[0] != v[0] || u[1] != v[1]) {
            corners++;
          }
          icurr = i;
          curr = next;
          touched[icurr] = 1;
          found = 1;
          break;
        }
      }
      if (!found) {
        printf("Could not close contour :( \n");
        exit(1);
      }
    }
    cntr = 0;
    for (int i = 0; i < remaining; ++i) {
      if (!touched[i]) {
        fences[cntr++] = fences[i];
      }
    }
    memset(touched, 0, sizeof(int) * cntr);
    remaining = cntr;
  }
  free(touched);
  return corners;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Missing input file\n");
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    printf("Could not open file\n");
    return 1;
  }
  char *gardens = (char *)malloc(sizeof(char) * BUFFER_SIZE * BUFFER_SIZE);
  char buffer[BUFFER_SIZE];
  int n_col, n_row = 0;
  while (fgets(buffer, BUFFER_SIZE, file)) {
    n_col = strcspn(buffer, "\n");
    for (int i = 0; i < n_col; ++i) {
      gardens[n_row * n_col + i] = buffer[i];
    }
    n_row++;
  }
  fclose(file);

  gardens = (char *)realloc(gardens, sizeof(char) * n_row * n_col);
  int *visited = (int *)calloc(sizeof(int), n_row * n_col);
  Fence *fences = (Fence *)malloc(sizeof(Fence) * n_row * n_col);
  Plots *plots = createPlots(n_row, n_col, gardens);

  int area, perimeter, corners;
  int part1 = 0, part2 = 0;
  for (int y = 0; y < plots->ny; ++y) {
    for (int x = 0; x < plots->nx; ++x) {
      if (visited[y * plots->nx + x]) {
        continue;
      }
      perimeter = area = 0;
      computePrice(plots, plots->grid[y][x], x, y, visited, &area, &perimeter,
                   fences);
      part1 += area * perimeter;
      corners = countCorners(fences, perimeter);
      part2 += area * corners;
    }
  }

  printf("Part 1: %d\n", part1);
  printf("Part 2: %d\n", part2);

  freePlots(plots);
  free(visited);
  free(fences);
  return 0;
}
