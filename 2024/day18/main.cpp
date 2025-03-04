#include <algorithm>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

void fill_grid(char **grid, int size, vector<tuple<int, int>> pixels,
               int n_pixel) {
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      grid[i][j] = '.';
    }
  }
  int x, y;
  for (int i = 0; i < n_pixel; ++i) {
    tie(x, y) = pixels[i];
    grid[y][x] = '#';
  }
}

bool solve(char **grid, int size, int *n_step) {
  tuple<int, int> MOVES[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
  vector<tuple<int, int, int>> queue = {tuple(0, 0, 0)};
  vector<tuple<int, int>> visited = {};
  tuple<int, int> pos = {};
  int x, y, steps;
  int ibest = 0;
  int best;
  while (!queue.empty()) {
    best = std::numeric_limits<int>::max();
    ibest = -1;
    for (int i = 0; i < queue.size(); ++i) {
      tie(x, y, steps) = queue[i];
      if (steps < best) {
        best = steps;
        ibest = i;
      }
    }
    tie(x, y, steps) = queue[ibest];
    queue.erase(queue.begin() + ibest);
    pos = {x, y};
    if (std::find(visited.begin(), visited.end(), pos) != visited.end()) {
      continue;
    }
    visited.push_back(pos);
    int dx, dy, xnew, ynew;
    for (int i = 0; i < 4; ++i) {
      tie(dx, dy) = MOVES[i];
      ynew = y + dy;
      xnew = x + dx;
      if (xnew < 0 || xnew >= size || ynew < 0 || ynew >= size) {
        continue;
      }
      if (grid[ynew][xnew] == '#') {
        continue;
      }
      if (xnew == size - 1 && ynew == size - 1) {
        *n_step = steps + 1;
        return true;
      }
      queue.push_back({xnew, ynew, steps + 1});
    }
  }
  return false;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <number>\n", argv[0]);
    return 1;
  }
  string filename = argv[1];
  int size, n_pixel;
  if (!filename.compare("input")) {
    size = 71;
    n_pixel = 1024;
  } else if (!filename.compare("example")) {
    size = 7;
    n_pixel = 12;
  } else {
    throw std::runtime_error("Unexpected file " + filename);
  }

  vector<tuple<int, int>> pixels = {};
  ifstream file(argv[1]);
  string line;
  while (getline(file, line)) {
    int x, y;
    sscanf(line.c_str(), "%d,%d", &x, &y);
    pixels.push_back({x, y});
  }
  // Initialize grid
  char *elem = (char *)malloc(sizeof(char) * size * (size + 1));
  char **grid = (char **)malloc(sizeof(char *) * size);
  grid[0] = elem + size;
  for (int i = 1; i < size; i++) {
    grid[i] = grid[i - 1] + size;
  }
  // Part 1
  fill_grid(grid, size, pixels, n_pixel);
  int n_step = -1;
  if (solve(grid, size, &n_step)) {
    printf("Path found in %d steps\n", n_step);
  }

  // Part 2
  int start = n_pixel;
  int stop = pixels.size();

  while (stop - start > 1) {
    int mid = (start + stop) / 2;
    fill_grid(grid, size, pixels, mid + 1);
    if (solve(grid, size, &n_step)) {
      start = mid + 1;
    } else {
      stop = mid;
    }
  }
  int x, y;
  tie(x, y) = pixels[stop];
  printf("Maze becomes unsolveable with pixel %d,%d\n", x, y);

  // Free memory
  free(grid);
  free(elem);

  return 0;
}
