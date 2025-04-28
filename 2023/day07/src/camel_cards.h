#ifndef CAMEL_CARDS_H
#define CAMEL_CARDS_H

#define N_CARD 13

enum Card {
  Two,
  Three,
  Four,
  Five,
  Six,
  Seven,
  Eight,
  Nine,
  Ten,
  Jack,
  Queen,
  King,
  Ace,
};

enum StrengthType {
  HighCard,
  OnePair,
  TwoPair,
  ThreeOfAKind,
  FullHouse,
  FourOfAKind,
  FiveOfAKind
};

extern const char symbols[N_CARD];
extern const int baseValues[N_CARD];

typedef struct {
  enum Card card;
  int value, count;
} CardCount;

typedef struct {
  int size, bid;
  int cardValues[5];
  enum StrengthType strength;
  CardCount hand[5];
} Hand;

typedef struct {
  int capacity, size;
  Hand *hands;
} HandList;

enum Card parseCard(char symbol);

int compareHands(const void *a, const void *b);
Hand *parseHand(const char *line);
HandList *createHandList(int capacity);
void addHand(HandList *list, Hand *hand);
int computeJokerScore(HandList *list);
void sortHandList(HandList *list);
int computeScore(HandList *list);
void freeHandList(HandList *list);

#endif
