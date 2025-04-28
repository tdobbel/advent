#include "camel_cards.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char symbols[N_CARD] = {'2', '3', '4', '5', '6', '7', '8',
                              '9', 'T', 'J', 'Q', 'K', 'A'};

const int baseValues[N_CARD] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

enum Card parseCard(char symbol) {
  for (int i = 0; i < N_CARD; i++) {
    if (symbols[i] == symbol) {
      return i;
    }
  }
  return -1; // Invalid card
}

int compareCounters(const void *a, const void *b) {
  CardCount *ccA = (CardCount *)a;
  CardCount *ccB = (CardCount *)b;
  if (ccA->count == ccB->count)
    return ccB->value - ccA->value;
  return ccB->count - ccA->count;
}

int compareHands(const void *a, const void *b) {
  Hand *handA = (Hand *)a;
  Hand *handB = (Hand *)b;
  if (handA->strength != handB->strength)
    return handA->strength - handB->strength;
  for (int i = 0; i < 5; ++i) {
    if (handA->cardValues[i] != handB->cardValues[i])
      return handA->cardValues[i] - handB->cardValues[i];
  }
  return 0;
}

static void sortHand(Hand *hand) {
  qsort(hand->hand, hand->size, sizeof(CardCount), compareCounters);
}

void setHandStrength(Hand *self) {
  if (self->hand[0].count == 5) {
    self->strength = FiveOfAKind;
  } else if (self->hand[0].count == 4) {
    self->strength = FourOfAKind;
  } else if (self->hand[0].count == 3) {
    if (self->hand[1].count == 2) {
      self->strength = FullHouse;
    } else {
      self->strength = ThreeOfAKind;
    }
  } else if (self->hand[0].count == 2) {
    if (self->hand[1].count == 2) {
      self->strength = TwoPair;
    } else {
      self->strength = OnePair;
    }
  } else {
    self->strength = HighCard;
  }
}

Hand *parseHand(const char *line) {
  char cards[6];
  Hand *hand = malloc(sizeof(Hand));
  sscanf(line, "%5s %d", cards, &hand->bid);
  hand->size = 0;
  for (int i = 0; i < 5; ++i) {
    enum Card card = parseCard(cards[i]);
    hand->cardValues[i] = baseValues[card];
    int found = 0;
    CardCount *cc = NULL;
    for (int j = 0; j < hand->size; ++j) {
      cc = &hand->hand[j];
      if (cc->card == card) {
        cc->count++;
        found = 1;
        break;
      }
    }
    if (found)
      continue;
    cc = &hand->hand[hand->size++];
    cc->card = card;
    cc->value = baseValues[card];
    cc->count = 1;
  }
  sortHand(hand);
  setHandStrength(hand);
  return hand;
}

static void printHand(const Hand *hand) {
  printf("Hand: ");
  for (int i = 0; i < hand->size; ++i) {
    const CardCount *cc = &hand->hand[i];
    printf("%d x %c,", cc->count, symbols[cc->card]);
  }
  printf(" Strength: %d, Bid: %d\n", hand->strength, hand->bid);
}

HandList *createHandList(int capacity) {
  HandList *list = malloc(sizeof(HandList));
  list->capacity = capacity;
  list->size = 0;
  list->hands = malloc(capacity * sizeof(Hand));
  return list;
}

void addHand(HandList *list, Hand *hand) {
  if (list->size == list->capacity) {
    list->capacity *= 2;
    list->hands = realloc(list->hands, list->capacity * sizeof(Hand));
  }
  list->hands[list->size++] = *hand;
  free(hand);
}

void freeHandList(HandList *list) {
  free(list->hands);
  free(list);
}

void sortHandList(HandList *list) {
  qsort(list->hands, list->size, sizeof(Hand), compareHands);
}

int computeScore(HandList *list) {
  int score = 0;
  for (int i = 0; i < list->size; ++i) {
    score += (i + 1) * list->hands[i].bid;
  }
  return score;
}

int computeJokerScore(HandList *list) {
  for (int i = 0; i < list->size; ++i) {
    Hand *hand = &list->hands[i];
    int jokerValue = baseValues[Jack];
    // Set value of J to 1
    for (int j = 0; j < 5; ++j) {
      if (hand->cardValues[j] == jokerValue) {
        hand->cardValues[j] = 1;
      }
    }
    // Add joker to the first card
    int iJoker = 0;
    while (iJoker < hand->size && hand->hand[iJoker].card != Jack) {
      iJoker++;
    }
    if (iJoker < hand->size && hand->size > 1) {
      int imax = 0;
      while (hand->hand[imax].card == Jack) {
        imax++;
      }
      hand->hand[imax].count += hand->hand[iJoker].count;
      hand->hand[iJoker].count = 0;
    }
    sortHand(hand);
    setHandStrength(hand);
  }
  sortHandList(list);
  return computeScore(list);
}
