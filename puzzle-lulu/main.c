#include <stdio.h>
#include <stdlib.h>

#define STRING_IMPLEMENTATION
#include "string8.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

#define N_LETTER 26

typedef struct {
  string8 word;
  u8 counter[N_LETTER];
} c_word;

void count_letters(string8 word, u8 *counter) {
  for (u64 i = 0; i < word.size; ++i) {
    char c = tolower(word.str[i]);
    if (c < 'a' || c > 'z')
      continue;
    counter[c - 'a']++;
  }
}

b8 ispossible(u8 *ref, u8 *counter) {
  for (int i = 0; i < 26; ++i) {
    if (counter[i] > ref[i])
      return 0;
  }
  return 1;
}

b8 matches(u8 *c1, u8 *c2) {
  for (int i = 0; i < 26; ++i) {
    if (c1[i] != c2[i])
      return 0;
  }
  return 1;
}

int main() {
  string8 puzzle = STR8_LIT("cryestmotolsns");
  u8 refcount[26] = {0};
  count_letters(puzzle, refcount);

  string8 file = {0};
  str_read_file(NULL, &file, "/usr/share/dict/words");

  vector *line_vec = VEC_CREATE(string8);
  str_split(line_vec, file, STR8_LIT("\n"));
  string8 *lines = (string8 *)line_vec->data;

  vector *candidates = VEC_CREATE(c_word);

  for (u64 i = 0; i < line_vec->size && lines[i].size > 0; ++i) {
    c_word candidate = (c_word){.word = lines[i], .counter = {0}};
    count_letters(candidate.word, candidate.counter);
    if (ispossible(refcount, candidate.counter))
      VEC_PUSH(candidates, c_word, candidate);
  }

  c_word *words = (c_word *)candidates->data;

  for (u64 i = 0; i < candidates->size - 1; ++i) {
    u8 new_ref[N_LETTER] = {0};
    for (u64 k = 0; k < N_LETTER; ++k) {
      new_ref[k] = refcount[k] - words[i].counter[k];
    }
    for (u64 j = i + 1; j < candidates->size; ++j) {
      if (matches(words[j].counter, new_ref))
        printf(STR8_FMT " " STR8_FMT "\n", STR8_UNWRAP(words[i].word),
               STR8_UNWRAP(words[j].word));
    }
  }

  vector_free(line_vec);
  vector_free(candidates);
  free(file.str);

  return EXIT_SUCCESS;
}
