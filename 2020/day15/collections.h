#ifndef _COLLECTIONS_H_
#define _COLLECTIONS_H_

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint8_t u8;
typedef u8 b8;

#define SEED 0x00000000
#define BASE_CAPACITY 16
#define DEFAULT_MAX_LOAD_FACTOR 60

typedef struct {
  u64 elem_size, capacity, size;
  void *data;
} vector;

vector *vector_create(u64 elem_size);
void vector_free(vector *vec);
void *vector_get(vector *vec, u64 index);
void *vector_append_get(vector *vec);

static u32 murmur_32_scramble(u32 k);
u32 murmur3_32(const u8 *key, u32 len, u32 seed);

struct hm_node {
  u32 hash;
  u8 *kv;
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
struct hm_node *find_key(struct hm_node *bucket, const void *target, u32 ctx,
                         b8 (*cmp_fun)(const void *, const void *, u32));

static u32 ensure_pow2(u32 cap);

hash_map *hm_init(u32 capacity, u32 key_size, u32 value_size);
kv_entry hm_get_entry(hash_map *hm, const void *key);
kv_entry hm_get_entry_with_hash(hash_map *hm, const void *key, u32 hash);
kv_entry hm_get_or_put(hash_map *hm, const void *key);
void grow_if_needed(hash_map *hm);
b8 hm_get(hash_map *hm, const void *key, void *value_ptr);
void hm_put(hash_map *hm, const void *key, const void *value);
void hm_flush(hash_map *hm);
void hm_deinit(hash_map *hm);

typedef struct {
  hash_map *hmap;
  u32 index, bucket;
  void *key_ptr;
  void *value_ptr;
  struct hm_node *entry;
} kv_iterator;

kv_iterator hm_iterator(hash_map *hm);
b8 get_next(kv_iterator *kv_iter);

typedef struct {
  u8 *str;
  u64 size;
} string8;

string8 clone_to_string8(const char *s);

typedef struct {
  hash_map *hm;
} str_hash_map;

b8 streql(const void *a, const void *b, u32 size);
str_hash_map *shm_init(u32 capacity, u32 value_size);
kv_entry shm_get_entry(str_hash_map *shm, const char *key);
kv_entry shm_get_entry_with_hash(hash_map *hm, string8 key, u32 hash);
kv_entry shm_get_or_put(str_hash_map *shm, const char *key);
b8 shm_get(str_hash_map *shm, const char *key, void *value_ptr);
void shm_put(str_hash_map *shm, const char *key, void *value);
void shm_deinit(str_hash_map *shm);

#ifdef VECTOR_IMPLEMENTATION

vector *vector_create(u64 elem_size) {
  vector *vec = (vector *)malloc(sizeof(vector));
  vec->size = 0;
  vec->elem_size = elem_size;
  vec->capacity = BASE_CAPACITY;
  vec->data = malloc(BASE_CAPACITY * elem_size);
  return vec;
}

void *vector_get(vector *vec, u64 index) {
  assert(vec && index < vec->size);
  return (void *)((char *)vec->data + index * vec->elem_size);
}

void *vector_append_get(vector *vec) {
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, vec->capacity * vec->elem_size);
  }
  return vector_get(vec, vec->size++);
}

void vector_free(vector *vec) {
  if (!vec)
    return;
  free(vec->data);
  free(vec);
}

#define VEC_CREATE(T) vector_create(sizeof(T))
#define VEC_PUSH(vec, T, x) (*(T *)vector_append_get((vec)) = (x))

#endif

#ifdef HASHMAP_IMPLEMENTATION

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

static u32 ensure_pow2(u32 cap) {
  u32 k = 1;
  while (k < cap) {
    k <<= 1;
  }
  return k;
}

hash_map *hm_init(u32 capacity, u32 key_size, u32 value_size) {
  hash_map *hm = (hash_map *)malloc(sizeof(hash_map));
  capacity = ensure_pow2(capacity);
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

struct hm_node *find_key(struct hm_node *bucket, const void *target, u32 ctx,
                         b8 (*cmp_fun)(const void *, const void *, u32)) {
  struct hm_node *node = bucket;
  while (node) {
    if (cmp_fun(node->kv, target, ctx)) {
      return node;
    }
    node = node->next;
  }
  return NULL;
}

kv_entry hm_get_entry_with_hash(hash_map *hm, const void *key, u32 hash) {
  u32 ibkt = hash & (hm->capacity - 1);
  struct hm_node *bucket = hm->buckets[ibkt];
  struct hm_node *node = find_key(bucket, key, hm->key_size, eql);
  if (node) {
    return (kv_entry){.found_existing = 1,
                      .key_ptr = node->kv,
                      .value_ptr = node->kv + hm->key_size};
  }
  return (kv_entry){.found_existing = 0, .key_ptr = NULL, .value_ptr = NULL};
}

kv_entry hm_get_entry(hash_map *hm, const void *key) {
  u32 hash = murmur3_32((u8 *)key, hm->key_size, SEED);
  return hm_get_entry_with_hash(hm, key, hash);
}

void grow_if_needed(hash_map *hm) {
  u32 load = (100 * hm->size) / hm->capacity;
  if (load < DEFAULT_MAX_LOAD_FACTOR)
    return;
  u32 new_cap = hm->capacity * 2;
  struct hm_node **buckets =
      (struct hm_node **)malloc(new_cap * sizeof(struct hm_node *));
  for (u32 i = 0; i < new_cap; ++i) {
    buckets[i] = NULL;
  }
  for (u32 i = 0; i < hm->capacity; ++i) {
    struct hm_node *node = hm->buckets[i];
    struct hm_node *tmp;
    while (node) {
      u32 ibkt = node->hash & (new_cap - 1);
      tmp = node->next;
      node->next = buckets[ibkt];
      buckets[ibkt] = node;
      node = tmp;
    }
  }
  free(hm->buckets);
  hm->buckets = buckets;
  hm->capacity = new_cap;
}

kv_entry hm_get_or_put(hash_map *hm, const void *key) {
  u32 hash = murmur3_32((u8 *)key, hm->key_size, SEED);
  kv_entry found = hm_get_entry_with_hash(hm, key, hash);
  if (found.found_existing) {
    return found;
  }
  grow_if_needed(hm);
  u32 ibkt = hash & (hm->capacity - 1);
  hm->size++;
  struct hm_node *new_node = (struct hm_node *)malloc(sizeof(struct hm_node));
  new_node->hash = hash;
  new_node->kv = (u8 *)malloc(hm->key_size + hm->value_size);
  memcpy(new_node->kv, key, hm->key_size);
  new_node->next = hm->buckets[ibkt];
  hm->buckets[ibkt] = new_node;
  return (kv_entry){.found_existing = 0,
                    .key_ptr = new_node->kv,
                    .value_ptr = new_node->kv + hm->key_size};
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

void hm_flush(hash_map *hm) {
  for (u32 i = 0; i < hm->capacity; ++i) {
    struct hm_node *entry = hm->buckets[i];
    struct hm_node *tmp;
    while (entry) {
      free(entry->kv);
      tmp = entry->next;
      free(entry);
      entry = tmp;
    }
  }
  hm->size = 0;
}

void hm_deinit(hash_map *hm) {
  hm_flush(hm);
  free(hm->buckets);
  free(hm);
}

#define HM_CREATE(K, V) hm_init(BASE_CAPACITY, sizeof(K), sizeof(V));
#define HM_PUT(hm, K, k, V, v)                                                 \
  {                                                                            \
    K key = (k);                                                               \
    V value = (v);                                                             \
    hm_put((hm), &key, &value);                                                \
  }

kv_iterator hm_iterator(hash_map *hm) {
  return (kv_iterator){.hmap = hm, .index = 0, .bucket = 0, .entry = NULL};
}

b8 get_next(kv_iterator *kvi) {
  if (kvi->index == kvi->hmap->size) {
    return 0;
  }
  if (kvi->index == 0) {
    kvi->entry = kvi->hmap->buckets[0];
  } else {
    kvi->entry = kvi->entry->next;
  }
  kvi->index++;
  while (kvi->entry == NULL) {
    kvi->bucket++;
    kvi->entry = kvi->hmap->buckets[kvi->bucket];
  }
  kvi->value_ptr = kvi->entry->kv;
  kvi->key_ptr = kvi->entry->kv + kvi->hmap->key_size;
  return 1;
}

#define STR8_LIT(s) ((string8){.str = (u8 *)(s), .size = strlen((s))})
#define STR8_FMT "%.*s"
#define STR8_UNWRAP(s) (int)(s).size, (char *)(s).str

b8 streql(const void *a, const void *b, u32 size) {
  string8 *sa = (string8 *)a;
  string8 *sb = (string8 *)b;
  if (sa->size != size || sb->size != size)
    return 0;
  return eql(sa->str, sb->str, sb->size);
}

str_hash_map *shm_init(u32 capacity, u32 value_size) {
  str_hash_map *shm = (str_hash_map *)malloc(sizeof(str_hash_map));
  shm->hm = hm_init(capacity, sizeof(string8), value_size);
  return shm;
}

kv_entry shm_get_entry_with_hash(hash_map *hm, string8 key, u32 hash) {
  u32 ibkt = hash & (hm->capacity - 1);
  struct hm_node *bucket = hm->buckets[ibkt];
  struct hm_node *node = find_key(bucket, &key, key.size, streql);
  if (node) {
    return (kv_entry){.found_existing = 1,
                      .key_ptr = node->kv,
                      .value_ptr = node->kv + hm->key_size};
  }
  return (kv_entry){.found_existing = 0, .key_ptr = NULL, .value_ptr = NULL};
}

kv_entry shm_get_entry(str_hash_map *shm, const char *key) {
  string8 key8 = STR8_LIT(key);
  u32 hash = murmur3_32(key8.str, key8.size, SEED);
  return shm_get_entry_with_hash(shm->hm, key8, hash);
}

string8 clone_to_string8(const char *s) {
  u64 size = strlen(s);
  u8 *str = (u8 *)malloc(strlen(s));
  memcpy(str, s, size);
  return (string8){.str = str, .size = size};
}

kv_entry shm_get_or_put(str_hash_map *shm, const char *key) {
  string8 key8 = STR8_LIT(key);
  u32 hash = murmur3_32(key8.str, key8.size, SEED);
  kv_entry found = shm_get_entry_with_hash(shm->hm, key8, hash);
  if (found.found_existing)
    return found;
  grow_if_needed(shm->hm);
  key8 = clone_to_string8(key);
  u32 ibkt = hash & (shm->hm->capacity - 1);
  shm->hm->size++;
  struct hm_node *new_node = (struct hm_node *)malloc(sizeof(struct hm_node));
  new_node->hash = hash;
  new_node->kv = (u8 *)malloc(sizeof(string8) + shm->hm->value_size);
  memcpy(new_node->kv, &key8, sizeof(string8));
  new_node->next = shm->hm->buckets[ibkt];
  shm->hm->buckets[ibkt] = new_node;
  return (kv_entry){.found_existing = 0,
                    .key_ptr = new_node->kv,
                    .value_ptr = new_node->kv + shm->hm->key_size};
}

b8 shm_get(str_hash_map *shm, const char *key, void *value_ptr) {
  kv_entry entry = shm_get_entry(shm, key);
  if (!entry.found_existing) {
    return 0;
  }
  memcpy(value_ptr, entry.value_ptr, shm->hm->value_size);
  return 1;
}

void shm_put(str_hash_map *shm, const char *key, void *value) {
  kv_entry entry = shm_get_or_put(shm, key);
  memcpy(entry.value_ptr, value, shm->hm->value_size);
}

void shm_deinit(str_hash_map *shm) {
  kv_iterator kvi = hm_iterator(shm->hm);
  while (get_next(&kvi)) {
    string8 *s = (string8 *)kvi.key_ptr;
    free(s->str);
  }
  hm_deinit(shm->hm);
  free(shm);
}

#define SHM_CREATE(V) shm_init(BASE_CAPACITY, sizeof(V));

#endif
#endif
