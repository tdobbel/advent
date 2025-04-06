#ifndef LAN_H
#define LAN_H

typedef struct {
  int size, capacity;
  int *values;
} IntVector;

typedef struct {
  int size, capacity;
  char **names;
} NameMap;

typedef struct {
  int n;
  NameMap *nmap;
  int **adjacency;
} Network;

NameMap *createNameMap(int capacity);
void freeNameMap(NameMap *nm);

IntVector *createIntVector(int capacity);
void push(IntVector *v, int value);
int contains(IntVector *v, int value);
void freeIntVector(IntVector *v);

int countTriangles(Network *network);
int findMaxClique(Network *nerwork, char *clique);

Network *readNetwork(const char *filename);
void freeNetwork(Network *network);

#endif
