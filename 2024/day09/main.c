#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void update_left(const int *block_start, int *block_left, int *ileft) {
  while (*ileft >= block_start[*block_left + 1]) {
    (*block_left) += 2;
    *ileft = block_start[*block_left];
  }
}

void update_right(const int *block_start, int *block_right, int *iright) {
  while (*iright < block_start[*block_right]) {
    (*block_right) -= 2;
    *iright = block_start[*block_right + 1] - 1;
  }
}

void print_disk(const int *disk, int n) {
  for (int i = 0; i < n; i++) {
    printf("%c ", disk[i] < 0 ? '.' : '0' + disk[i]);
  }
  printf("\n");
}

void initialize_blocks(int *disk, const int *block_start, int n_block) {
  memset(disk, -1, block_start[n_block] * sizeof(int));
  for (int i = 0; i < n_block; i += 2) {
    for (int j = block_start[i]; j < block_start[i + 1]; j++) {
      disk[j] = i / 2;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing input file\n");
    return -1;
  }

  FILE *file = fopen(argv[1], "r");
  char buffer[20000];
  fgets(buffer, 20000, file);
  int n = strcspn(buffer, "\n");
  int *block_start = (int *)malloc((n + 1) * sizeof(int));
  int *block_size = (int *)malloc(n * sizeof(int));
  block_start[0] = 0;
  int k = 0;
  for (int i = 0; i < n; i++) {
    block_size[i] = buffer[i] - '0';
    block_start[i + 1] = block_start[i] + block_size[i];
  }
  fclose(file);

  int *disk = (int *)malloc(block_start[n] * sizeof(int));
  initialize_blocks(disk, block_start, n);

  int block_right = n % 2 ? n - 1 : n - 2;
  int iright = block_start[block_right + 1] - 1;
  update_right(block_start, &block_right, &iright);
  int block_left = 1;
  int ileft = block_start[block_left];

  while (ileft < iright) {
    disk[ileft] = disk[iright];
    disk[iright] = -1;
    ileft++;
    iright--;
    update_left(block_start, &block_left, &ileft);
    update_right(block_start, &block_right, &iright);
  }

  // Part 1
  uint64_t sol1 = 0;
  for (int i = 0; i < block_start[n] && disk[i] >= 0; i++) {
    sol1 += (uint64_t)(disk[i] * i);
  }
  printf("Part 1: %lu\n", sol1);

  // Part 2
  initialize_blocks(disk, block_start, n);
  block_right = n % 2 ? n - 1 : n - 2;
  int value;
  for (int iright = block_right; iright >= 0; iright -= 2) {
    int size_right = block_size[iright];
    if (size_right == 0)
      continue;
    value = disk[block_start[iright]];
    for (ileft = 1; ileft < iright; ileft += 2) {
      if (block_size[ileft] < size_right)
        continue;
      for (int k = 0; k < size_right; k++) {
        disk[block_start[ileft] + k] = value;
        disk[block_start[iright] + k] = -1;
      }
      block_start[ileft] += size_right;
      block_size[ileft] -= size_right;
      break;
    }
  }
  uint64_t sol2 = 0;
  for (int i = 0; i < block_start[n]; ++i) {
    if (disk[i] >= 0)
      sol2 += (uint64_t)(disk[i] * i);
  }
  printf("Part 2: %lu\n", sol2);

  free(block_start);
  free(block_size);
  free(disk);

  return 0;
}
