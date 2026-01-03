#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 524

typedef enum { false, true } bool;

typedef struct {
  int board[5][5];
  bool marked[5][5];
} BingoBoard;

BingoBoard *createBoard(int board[5][5]) {
  BingoBoard *bb = malloc(sizeof(BingoBoard));
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      bb->board[i][j] = board[i][j];
      bb->marked[i][j] = false;
    }
  }
  return bb;
}

void markNumber(BingoBoard *bb, int num) {
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      if (bb->board[i][j] == num) {
        bb->marked[i][j] = true;
      }
    }
  }
}

int winningRow(BingoBoard *bb, int row) {
  for (int col = 0; col < 5; ++col) {
    if (bb->marked[row][col] == false) {
      return 0;
    }
  }
  return 1;
}

int winningCol(BingoBoard *bb, int col) {
  for (int row = 0; row < 5; ++row) {
    if (bb->marked[row][col] == false) {
      return 0;
    }
  }
  return 1;
}

int winningBoard(BingoBoard *bb) {
  for (int i = 0; i < 5; i++) {
    if (winningCol(bb, i) || winningRow(bb, i)) {
      return 1;
    }
  }
  return 0;
}

int unmarkedSum(BingoBoard *bb) {
  int sum = 0;
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      if (bb->marked[i][j] == false) {
        sum += bb->board[i][j];
      }
    }
  }
  return sum;
}

void displayBoard(BingoBoard *bb) {
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      if (bb->marked[i][j] == false) {
        printf(" %2d  ", bb->board[i][j]);
      } else {
        printf(">%2d< ", bb->board[i][j]);
      }
    }
    printf("\n");
  }
  printf("\n");
}

struct Node {
  BingoBoard *bingoBoard;
  struct Node *next;
};

typedef struct {
  struct Node *head;
  struct Node *tail;
} LinkedList;

void appendToList(LinkedList *ll, int board[5][5]) {
  struct Node *node = malloc(sizeof(struct Node));
  node->bingoBoard = createBoard(board);
  if (ll->head == NULL) {
    ll->head = node;
    ll->tail = node;
  } else {
    struct Node *tmp = ll->tail;
    tmp->next = node;
    ll->tail = node;
  }
}

void freeList(LinkedList *ll) {
  struct Node *node = ll->head;
  while (node) {
    struct Node *next = node->next;
    free(node->bingoBoard);
    free(node);
    node = next;
  }
  free(ll);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "Error opening file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  char line[BUFFER_SIZE];

  fgets(line, BUFFER_SIZE, fp);
  int n = strcspn(line, "\n");
  int *numbers = malloc(BUFFER_SIZE * sizeof(int));
  int nNum = 0;
  int current = 0;
  for (int i = 0; i < n + 1; ++i) {
    if (line[i] == ',' || line[i] == '\n') {
      numbers[nNum++] = current;
      current = 0;
      continue;
    }
    current = current * 10 + line[i] - '0';
  }
  numbers = realloc(numbers, nNum * sizeof(int));
  fgets(line, BUFFER_SIZE, fp);
  LinkedList *ll = malloc(sizeof(LinkedList));
  int row = 0;
  int col;
  int board[5][5];
  while (fgets(line, BUFFER_SIZE, fp)) {
    if (row == 5) {
      row = 0;
      continue;
    }
    col = 0;
    n = strcspn(line, "\n");
    current = 0;
    for (int i = 0; i < n + 1; ++i) {
      if ((i + 1) % 3 == 0) {
        board[row][col++] = current;
        current = 0;
        continue;
      }
      if (line[i] == ' ') {
        continue;
      }
      current = current * 10 + line[i] - '0';
    }
    row++;
    if (row == 5) {
      appendToList(ll, board);
    }
  }

  int part1 = -1;
  int part2;

  for (int inum = 0; inum < nNum; ++inum) {
    if (ll->head == NULL)
      break;
    struct Node *node = ll->head;
    struct Node *prev = NULL;
    struct Node *next = NULL;
    while (node) {
      markNumber(node->bingoBoard, numbers[inum]);
      next = node->next;
      if (winningBoard(node->bingoBoard)) {
        part2 = numbers[inum] * unmarkedSum(node->bingoBoard);
        if (part1 == -1) {
          part1 = part2;
        }
        free(node->bingoBoard);
        free(node);
        if (prev == NULL) {
          ll->head = next;
        } else {
          prev->next = next;
        }
        if (next == NULL) {
          ll->tail = prev;
        }
        node = next;
      } else {
        prev = node;
        node = next;
      }
    }
  }

  printf("Part 1: %d\n", part1);
  printf("Part 2: %d\n", part2);

  fclose(fp);
  free(numbers);
  freeList(ll);

  return EXIT_SUCCESS;
}
