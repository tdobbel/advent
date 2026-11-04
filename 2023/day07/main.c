#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

typedef enum {
  HighCard,
  OnePair,
  TwoPair,
  ThreeOfAKind,
  FullHouse,
  FourOfAKind,
  FiveOfAKind
} HandType;

typedef struct {
  HandType hand_type;
  u8 cards[5];
  u32 strength[5];
  u32 bid;
} Hand;

u32 card_to_strength(u8 card);
u32 card_to_strength_joker(u8 card);
HandType counter_to_hand_type(u8 *counter);
void set_hand_type(Hand *hand);
void set_hand_type_joker(Hand *hand);
void update_hand_joker(Hand *hand);
u32 compute_total_winnings(Hand *hands, u64 n_hand);

int compare_u8(const void *a, const void *b) { return *(u8 *)b - *(u8 *)a; }
int compare_hand(const void *a, const void *b);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }
  string8 file = {0};
  str_read_file(NULL, &file, argv[1]);
  vector *line_vec = VEC_CREATE(string8);
  str_split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;

  vector *hand_vec = VEC_CREATE(Hand);

  string8 splitted[2] = {0};
  string8 space = STR8_LIT(" ");

  for (u32 i = 0; i < line_vec->size && lines[i].size > 0; ++i) {
    str_split_once(splitted, lines[i], space);
    assert(splitted[0].size == 5);
    Hand hand = {0};
    memcpy(hand.cards, splitted[0].str, 5);
    for (u32 ic = 0; ic < 5; ++ic) {
      hand.strength[ic] = card_to_strength(hand.cards[ic]);
    }
    hand.bid = str_parse_unsigned(splitted[1]);
    set_hand_type(&hand);

    VEC_PUSH(hand_vec, Hand, hand);
  }
  vector_free(line_vec);

  Hand *hands = (Hand *)hand_vec->data;

  u32 part1 = compute_total_winnings(hands, hand_vec->size);
  printf("Part 1: %u\n", part1);

  for (u32 i = 0; i < hand_vec->size; ++i) {
    update_hand_joker(&hands[i]);
  }
  u32 part2 = compute_total_winnings(hands, hand_vec->size);
  printf("Part 2: %u\n", part2);

  vector_free(hand_vec);
  free(file.str);

  return EXIT_SUCCESS;
}

u32 card_to_strength(u8 card) {
  if (card >= '2' && card <= '9')
    return card - '2';
  switch (card) {
  case 'T':
    return 8;
  case 'J':
    return 9;
  case 'Q':
    return 10;
  case 'K':
    return 11;
  case 'A':
    return 12;
  }
  fprintf(stderr, "Unknown card '%c'", card);
  exit(1);
}

u32 card_to_strength_joker(u8 card) {
  if (card >= '2' && card <= '9')
    return card - '1';
  switch (card) {
  case 'J':
    return 0;
  case 'T':
    return 9;
  case 'Q':
    return 10;
  case 'K':
    return 11;
  case 'A':
    return 12;
  }
  fprintf(stderr, "Unknown card '%c'", card);
  exit(1);
}

HandType counter_to_hand_type(u8 *counter) {
  qsort(counter, 13, sizeof(u8), compare_u8);
  switch (counter[0]) {
  case 5:
    return FiveOfAKind;
  case 4:
    return FourOfAKind;
  case 3:
    return counter[1] == 2 ? FullHouse : ThreeOfAKind;
  case 2:
    return counter[1] == 2 ? TwoPair : OnePair;
  default:
    return HighCard;
  }
}

void set_hand_type(Hand *hand) {
  u8 counter[13];
  memset(counter, 0, 13);
  for (u32 i = 0; i < 5; ++i) {
    counter[hand->strength[i]]++;
  }
  hand->hand_type = counter_to_hand_type(counter);
}

int compare_hand(const void *a, const void *b) {
  Hand *ha = (Hand *)a;
  Hand *hb = (Hand *)b;
  if (ha->hand_type != hb->hand_type) {
    return ha->hand_type - hb->hand_type;
  }
  for (u32 i = 0; i < 5; ++i) {
    if (ha->strength[i] != hb->strength[i])
      return ha->strength[i] - hb->strength[i];
  }
  return 0;
}

u32 compute_total_winnings(Hand *hands, u64 n_hand) {
  qsort(hands, n_hand, sizeof(Hand), compare_hand);
  u32 total = 0;
  for (u32 i = 0; i < n_hand; ++i) {
    total += (i + 1) * hands[i].bid;
  }
  return total;
}

void set_hand_type_joker(Hand *hand) {
  u8 counter[13];
  memset(counter, 0, 13);
  for (u32 i = 0; i < 5; ++i) {
    counter[hand->strength[i]]++;
  }
  if (counter[0] != 5) {
    u32 imax = 0;
    u8 vmax = 0;
    for (u32 i = 1; i < 13; ++i) {
      if (counter[i] > 0 && counter[i] >= vmax) {
        imax = i;
        vmax = counter[i];
      }
    }
    counter[imax] += counter[0];
    counter[0] = 0;
  }
  hand->hand_type = counter_to_hand_type(counter);
}

void update_hand_joker(Hand *hand) {
  for (u32 ic = 0; ic < 5; ++ic) {
    hand->strength[ic] = card_to_strength_joker(hand->cards[ic]);
  }
  set_hand_type_joker(hand);
}
