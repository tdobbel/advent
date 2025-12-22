#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 128

struct Node {
  char word[BUFFER_SIZE];
  int counter[26];
  struct Node *next;
};

typedef struct {
  int size;
  struct Node *head;
  struct Node *end;
} LinkedList;

LinkedList *create_list() {
  LinkedList *ll = malloc(sizeof(LinkedList));
  ll->size = 0;
  ll->head = NULL;
  ll->end = NULL;
  return ll;
}

void append(LinkedList *ll, char *name, int size, int counter[26]) {
  struct Node *node = malloc(sizeof(struct Node));
  memcpy(node->counter, counter, 26 * sizeof(int));
  memcpy(node->word, name, size);
  node->word[size] = '\0';
  node->next = NULL;
  if (ll->head == NULL) {
    ll->head = node;
    ll->end = node;
  } else {
    struct Node *prev = ll->end;
    prev->next = node;
    ll->end = node;
  }
  ll->size++;
}

void free_list(LinkedList *ll) {
  struct Node *node = ll->head;
  while (node) {
    struct Node *next = node->next;
    free(node);
    node = next;
  }
  free(ll);
}

void count_letters(char *word, int size, int *counter) {
  for (int i = 0; i < size; ++i) {
    char c = tolower(word[i]);
    if (c < 'a' || c > 'z')
      continue;
    counter[c - 'a']++;
  }
}

int ispossible(int *ref, int *counter) {
  for (int i = 0; i < 26; ++i) {
    if (counter[i] > ref[i])
      return 0;
  }
  return 1;
}

int matches(int *c1, int *c2) {
  for (int i = 0; i < 26; ++i) {
    if (c1[i] != c2[i])
      return 0;
  }
  return 1;
}

int main() {
  char buffer[BUFFER_SIZE];
  char *puzzle = "cryestmotolsns";
  int refcount[26] = {0};
  count_letters(puzzle, 14, refcount);

  FILE *fp = fopen("/usr/share/dict/words", "r");
  if (fp == NULL) {
    fprintf(stderr, "Could not open dictionary file");
    return EXIT_FAILURE;
  }
  LinkedList *ll = create_list();
  while (fgets(buffer, BUFFER_SIZE, fp)) {
    int size = strcspn(buffer, "\n");
    int counter[26] = {0};
    count_letters(buffer, size, counter);
    if (ispossible(refcount, counter)) {
      append(ll, buffer, size, counter);
    }
  }
  fclose(fp);

  struct Node *node1 = ll->head;
  int new_ref[26];
  while (node1 != ll->end) {
    memcpy(new_ref, refcount, 26 * sizeof(int));
    for (int i = 0; i < 26; ++i) {
      new_ref[i] -= node1->counter[i];
    }
    struct Node *node2 = node1->next;
    while (node2) {
      if (matches(new_ref, node2->counter)) {
        printf("%s %s\n", node1->word, node2->word);
      }
      node2 = node2->next;
    }
    node1 = node1->next;
  }

  free_list(ll);

  return EXIT_SUCCESS;
}
