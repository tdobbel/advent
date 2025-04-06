#include "lan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

NameMap *createNameMap(int capacity) {
  NameMap *nm = malloc(sizeof(NameMap));
  nm->size = 0;
  nm->capacity = capacity;
  nm->names = malloc(capacity * sizeof(char *));
  return nm;
}

int getNameIndex(NameMap *nm, const char *name) {
  for (int i = 0; i < nm->size; i++) {
    if (strcmp(nm->names[i], name) == 0) {
      return i;
    }
  }
  if (nm->size == nm->capacity) {
    nm->capacity *= 2;
    nm->names = realloc(nm->names, nm->capacity * sizeof(char *));
  }
  nm->names[nm->size] = malloc(strlen(name) + 1);
  strcpy(nm->names[nm->size], name);
  nm->size++;
  return nm->size - 1;
}

void freeNameMap(NameMap *nm) {
  for (int i = 0; i < nm->size; i++) {
    free(nm->names[i]);
  }
  free(nm->names);
  free(nm);
}

IntVector *createIntVector(int capacity) {
  IntVector *v = malloc(sizeof(IntVector));
  v->size = 0;
  v->capacity = capacity;
  v->values = malloc(capacity * sizeof(int));
  return v;
}

int contains(IntVector *v, int value) {
  for (int i = 0; i < v->size; i++) {
    if (v->values[i] == value)
      return 1;
  }
  return 0;
}

void freeIntVector(IntVector *v) {
  free(v->values);
  free(v);
}

void push(IntVector *v, int value) {
  if (v->size == v->capacity) {
    v->capacity *= 2;
    v->values = realloc(v->values, v->capacity * sizeof(int));
  }
  v->values[v->size] = value;
  v->size++;
}

Network *readNetwork(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Error opening file: %s\n", filename);
    return NULL;
  }

  NameMap *nameMap = createNameMap(150);
  IntVector *pairs = createIntVector(500);
  char buffer[256];
  char name1[3], name2[3];
  while (fgets(buffer, sizeof(buffer), file)) {
    buffer[strcspn(buffer, "\n")] = '\0';
    sscanf(buffer, "%2s-%2s", name1, name2);
    int i = getNameIndex(nameMap, name1);
    int j = getNameIndex(nameMap, name2);
    push(pairs, i);
    push(pairs, j);
  }
  fclose(file);

  Network *network = malloc(sizeof(Network));
  network->n = nameMap->size;
  network->nmap = nameMap;

  int *adj = calloc(sizeof(int), network->n * network->n);
  network->adjacency = malloc(network->n * sizeof(int *));
  network->adjacency[0] = adj;
  for (int i = 1; i < network->n; i++) {
    network->adjacency[i] = network->adjacency[i - 1] + network->n;
  }

  for (int k = 0; k < pairs->size; k += 2) {
    int i = pairs->values[k];
    int j = pairs->values[k + 1];
    network->adjacency[i][j] = 1;
    network->adjacency[j][i] = 1;
  }

  freeIntVector(pairs);

  return network;
}

void freeNetwork(Network *network) {
  free(network->adjacency[0]);
  free(network->adjacency);
  freeNameMap(network->nmap);
  free(network);
}

static int compareInt(const void *a, const void *b) {
  return (*(int *)a - *(int *)b);
}

static int hashTriangle(int nNode, int i, int j, int k) {
  int nodes[3] = {i, j, k};
  qsort(nodes, 3, sizeof(int), compareInt);
  return nodes[0] * nNode * nNode + nodes[1] * nNode + nodes[2];
}

int countTriangles(Network *network) {
  int result = 0;
  IntVector *found = createIntVector(100);
  for (int i = 0; i < network->n; i++) {
    char *name = network->nmap->names[i];
    if (name[0] != 't')
      continue;
    for (int j = 0; j < network->n; j++) {
      if (!network->adjacency[i][j])
        continue;
      for (int k = 0; k < network->n; k++) {
        if (!network->adjacency[j][k] || !network->adjacency[k][i]) {
          continue;
        }
        int hash = hashTriangle(network->n, i, j, k);
        if (!contains(found, hash)) {
          push(found, hash);
          result++;
        }
      }
    }
  }
  freeIntVector(found);
  return result;
}

static int compareName(const void *a, const void *b) {
  char *na = *(char **)a;
  char *nb = *(char **)b;
  return strcmp(na, nb);
}

int findMaxClique(Network *network, char *clique) {
  int nmax = 0;
  int start;
  int *nodes = malloc(network->n * sizeof(int));
  for (int i = 0; i < network->n; i++) {
    nodes[i] = i;
  }
  int prev = 0;
  int next = 1;
  while (next < network->n) {
    for (int i = next; i < network->n; ++i) {
      int add = 1;
      for (int j = prev; j < next; ++j) {
        if (!network->adjacency[nodes[i]][nodes[j]]) {
          add = 0;
          break;
        }
      }
      if (add) {
        int tmp = nodes[i];
        nodes[i] = nodes[next];
        nodes[next] = tmp;
        next++;
      }
    }
    int n = next - prev;
    if (n > nmax) {
      nmax = n;
      start = prev;
    }
    prev = next;
    next = prev + 1;
  }
  char **nodenames = malloc(nmax * sizeof(char *));
  for (int i = 0; i < nmax; i++) {
    nodenames[i] = malloc(3 * sizeof(char));
    strcpy(nodenames[i], network->nmap->names[nodes[start + i]]);
  }
  qsort(nodenames, nmax, sizeof(char *), compareName);
  sprintf(clique, "%s", nodenames[0]);
  free(nodenames[0]);
  for (int i = 1; i < nmax; i++) {
    sprintf(clique, "%s,%s", clique, nodenames[i]);
    free(nodenames[i]);
  }
  free(nodenames);
  free(nodes);

  return nmax;
}
