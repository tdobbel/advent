#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint8_t u8;
typedef u8 b8;

#define SEED 0x00000000
#define DEFAULT_CAPACITY 1024

static u32 murmur_32_scramble(u32 k);
u32 murmur3_32(const u8 *key, u32 len, u32 seed);

struct hm_node {
  void *key;
  void *value;
  struct hm_node *next;
};

typedef struct {
  b8 found_existing;
  void *key_ptr;
  void *value_ptr;
} kv_entry;

typedef struct {
  u32 capacity, size;
  u32 key_size;
  u32 value_size;
  struct hm_node **buckets;
} hash_map;

b8 eql(const void *a, const void *b, u32 size);

hash_map *hm_init(u32 capacity, u32 key_size, u32 value_size);
kv_entry hm_get_entry(hash_map *hm, const void *key);
kv_entry hm_get_or_put(hash_map *hm, const void *key);
b8 hm_get(hash_map *hm, const void *key, void *value_ptr);
void hm_put(hash_map *hm, const void *key, const void *value);
void hm_deinit(hash_map *hm);

typedef struct {
  hash_map *hmap;
  u32 index, bucket;
  struct hm_node *entry;
} kv_iterator;

kv_iterator hm_iterator(hash_map *hm);
b8 get_next(kv_iterator *kv_iter);

#define HM_CREATE(K, V) hm_init(DEFAULT_CAPACITY, sizeof(K), sizeof(V));
#define HM_PUT(hm, K, k, V, v)                                                 \
  {                                                                            \
    K key = (k);                                                               \
    V value = (v);                                                             \
    hm_put((hm), &key, &value);                                                \
  }

int main(void) {
  hash_map *hm = HM_CREATE(u64, u32);
  u64 keys[4] = {3, 42, 69, 1995};
  for (u32 i = 0; i < 4; ++i) {
    hm_put(hm, keys + i, &i);
  }
  kv_iterator kvi = hm_iterator(hm);
  while (get_next(&kvi)) {
    printf("%lu => %u\n", *(u64 *)kvi.entry->key, *(u32 *)kvi.entry->value);
  }
  hm_deinit(hm);
}

static inline u32 murmur_32_scramble(u32 k) {
  k *= 0xcc9e2d51;
  k = (k << 15) | (k >> 17);
  k *= 0x1b873593;
  return k;
}

// Implementation for little endian
u32 murmur3_32(const u8 *key, u32 len, u32 seed) {
  u32 h = seed;
  u32 k;
  // Read in groups of 4
  for (size_t i = len >> 2; i; i--) {
    memcpy(&k, key, sizeof(u32));
    key += sizeof(u32);
    h ^= murmur_32_scramble(k);
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;
  }
  // Read the rest.
  k = 0;
  for (size_t i = len & 3; i; i--) {
    k <<= 8;
    k |= key[i - 1];
  }
  // A swap is *not* necessary here because the preceding loop already
  // places the low bytes in the low places according to whatever endianness
  // we use. Swaps only apply when the memory is copied in a chunk.
  h ^= murmur_32_scramble(k);
  // Finalize
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

hash_map *hm_init(u32 capacity, u32 key_size, u32 value_size) {
  hash_map *hm = (hash_map *)malloc(sizeof(hash_map));
  hm->buckets = (struct hm_node **)malloc(capacity * sizeof(struct hm_node *));
  for (u32 i = 0; i < capacity; ++i) {
    hm->buckets[i] = NULL;
  }
  hm->capacity = capacity;
  hm->size = 0;
  hm->key_size = key_size;
  hm->value_size = value_size;
  return hm;
}

b8 eql(const void *a, const void *b, u32 size) {
  u8 *abytes = (u8 *)a;
  u8 *bbytes = (u8 *)b;
  for (u32 i = 0; i < size; ++i) {
    if (abytes[i] != bbytes[i])
      return 0;
  }
  return 1;
}

kv_entry hm_get_entry(hash_map *hm, const void *key) {
  u32 hash = murmur3_32((u8 *)key, hm->key_size, SEED);
  u32 ibkt = hash % hm->capacity;
  struct hm_node *node = hm->buckets[ibkt];
  while (node != NULL) {
    if (eql(key, node->key, hm->key_size)) {
      return (kv_entry){
          .found_existing = 1, .key_ptr = node->key, .value_ptr = node->value};
    }
    node = node->next;
  }
  return (kv_entry){.found_existing = 0, .key_ptr = NULL, .value_ptr = NULL};
}

kv_entry hm_get_or_put(hash_map *hm, const void *key) {
  u32 hash = murmur3_32((u8 *)key, hm->key_size, SEED);
  u32 ibkt = hash % hm->capacity;
  struct hm_node *node = hm->buckets[ibkt];
  while (node != NULL) {
    if (eql(key, node->key, hm->key_size)) {
      return (kv_entry){
          .found_existing = 1, .key_ptr = node->key, .value_ptr = node->value};
    }
    node = node->next;
  }
  hm->size++;
  struct hm_node *new_node = (struct hm_node *)malloc(sizeof(struct hm_node));
  new_node->key = malloc(hm->key_size);
  memcpy(new_node->key, key, hm->key_size);
  new_node->value = malloc(hm->value_size);
  new_node->next = hm->buckets[ibkt];
  hm->buckets[ibkt] = new_node;
  return (kv_entry){.found_existing = 0,
                    .key_ptr = new_node->key,
                    .value_ptr = new_node->value};
}

void hm_put(hash_map *hm, const void *key, const void *value) {
  kv_entry entry = hm_get_or_put(hm, key);
  memcpy(entry.value_ptr, value, hm->value_size);
}

b8 hm_get(hash_map *hm, const void *key, void *value_ptr) {
  kv_entry entry = hm_get_entry(hm, key);
  if (!entry.found_existing) {
    return 0;
  }
  memcpy(value_ptr, entry.value_ptr, hm->value_size);
  return 1;
}

void hm_deinit(hash_map *hm) {
  for (u32 i = 0; i < hm->capacity; ++i) {
    struct hm_node *entry = hm->buckets[i];
    struct hm_node *tmp;
    while (entry) {
      free(entry->key);
      free(entry->value);
      tmp = entry->next;
      free(entry);
      entry = tmp;
    }
  }
  free(hm->buckets);
  free(hm);
}

kv_iterator hm_iterator(hash_map *hm) {
  return (kv_iterator){.hmap = hm, .index = 0, .bucket = 0, .entry = NULL};
}

b8 get_next(kv_iterator *kv_iter) {
  if (kv_iter->index == kv_iter->hmap->size) {
    return 0;
  }
  if (kv_iter->index == 0) {
    kv_iter->entry = kv_iter->hmap->buckets[0];
  } else {
    kv_iter->entry = kv_iter->entry->next;
  }
  kv_iter->index++;
  while (kv_iter->entry == NULL) {
    kv_iter->bucket++;
    kv_iter->entry = kv_iter->hmap->buckets[kv_iter->bucket];
  }
  return 1;
};
