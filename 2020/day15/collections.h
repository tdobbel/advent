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
#define DEFAULT_MAX_LOAD_FACTOR 80

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

typedef struct {
  u32 key_size, value_size;
} hash_map_context;

typedef b8 (*eq_fn)(const hash_map_context ctx, const void *, const void *);

typedef struct {
  b8 found_existing;
  void *key_ptr;
  void *value_ptr;
} kv_entry;

typedef struct {
  u32 capacity, size;
  hash_map_context ctx;
  u8 *keys;
  u8 *values;
  b8 *isfree;
  eq_fn eq;
} hash_map;

b8 bytes_eql(const hash_map_context ctx, const void *a, const void *b);
b8 string8_eql(const hash_map_context ctx, const void *a, const void *b);

static u32 ensure_pow2(u32 cap);

hash_map *hm_init(u32 capacity, hash_map_context ctx, eq_fn eq);
u32 hm_get_index(hash_map *hm, const void *key);
kv_entry hm_get_entry(hash_map *hm, const void *key);
kv_entry hm_get_or_put(hash_map *hm, const void *key);
void grow_if_needed(hash_map *hm);
b8 hm_get(hash_map *hm, const void *key, void *value_ptr);
void hm_put(hash_map *hm, const void *key, const void *value);
void hm_put_assume_capacity(hash_map *hm, const void *key, const void *value);
void hm_deinit(hash_map *hm);

typedef struct {
  hash_map *hm;
  u32 cntr, indx;
  void *key_ptr;
  void *value_ptr;
} kv_iterator;

kv_iterator hm_iterator(hash_map *hm);
b8 get_next(kv_iterator *kv_iter);

typedef struct {
  u8 *str;
  u64 size;
} string8;

string8 to_string8(const char *s);

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

#define AUTO_HASHMAP(K, V)                                                     \
  hm_init(BASE_CAPACITY,                                                       \
          (hash_map_context){.key_size = sizeof(K), .value_size = sizeof(V)},  \
          bytes_eql);

#define STRING_HASHMAP(V)                                                      \
  hm_init(BASE_CAPACITY,                                                       \
          (hash_map_context){.key_size = sizeof(string8),                      \
                             .value_size = sizeof(V)},                         \
          string8_eql);

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

hash_map *hm_init(u32 capacity, hash_map_context ctx, eq_fn eq) {
  hash_map *hm = (hash_map *)malloc(sizeof(hash_map));
  u32 cap = ensure_pow2(capacity);
  hm->capacity = cap;
  hm->size = 0;
  hm->ctx = ctx;
  hm->keys = (u8 *)malloc(cap * ctx.key_size);
  hm->values = (u8 *)malloc(cap * ctx.value_size);
  hm->eq = eq;
  hm->isfree = (u8 *)malloc(cap);
  memset(hm->isfree, 1, cap);
  return hm;
}

b8 bytes_eql(const hash_map_context ctx, const void *a, const void *b) {
  u8 *abytes = (u8 *)a;
  u8 *bbytes = (u8 *)b;
  for (u32 i = 0; i < ctx.key_size; ++i) {
    if (abytes[i] != bbytes[i])
      return 0;
  }
  return 1;
}

u32 hm_get_index(hash_map *hm, const void *key) {
  u32 hash = murmur3_32((u8 *)key, hm->ctx.key_size, SEED);
  u32 indx = hash & (hm->capacity - 1);
  u32 limit = hm->capacity;
  while (!hm->isfree[indx] && limit > 0) {
    u8 *test_key = hm->keys + indx * hm->ctx.key_size;
    if (hm->eq(hm->ctx, test_key, key))
      return indx;
    indx = (indx + 1) & (hm->capacity - 1);
    limit--;
  }
  assert(limit > 0);
  return indx;
}

kv_entry hm_get_entry(hash_map *hm, const void *key) {
  u32 indx = hm_get_index(hm, key);
  if (hm->isfree[indx])
    return (kv_entry){.found_existing = 0, .key_ptr = NULL, .value_ptr = NULL};
  u8 *key_ptr = hm->keys + indx * hm->ctx.key_size;
  u8 *value_ptr = hm->keys + indx * hm->ctx.value_size;
  return (kv_entry){
      .found_existing = 1, .key_ptr = key_ptr, .value_ptr = value_ptr};
}

void hm_put_assume_capacity(hash_map *hm, const void *key, const void *value) {
  u32 indx = hm_get_index(hm, key);
  u8 *value_ptr = hm->values + indx * hm->ctx.value_size;
  memcpy(value_ptr, value, hm->ctx.value_size);
  if (hm->isfree[indx]) {
    u8 *key_ptr = hm->keys + indx * hm->ctx.key_size;
    memcpy(key_ptr, key, hm->ctx.key_size);
    hm->isfree[indx] = 0;
    hm->size++;
  }
}

void grow_if_needed(hash_map *hm) {
  u32 load = (100 * hm->size) / hm->capacity;
  if (load < DEFAULT_MAX_LOAD_FACTOR)
    return;
  u32 new_cap = hm->capacity * 2;
  hash_map *map = hm_init(new_cap, hm->ctx, hm->eq);
  for (u32 i = 0; i < hm->capacity; ++i) {
    if (hm->isfree[i])
      continue;
    u8 *key_ptr = hm->keys + i * hm->ctx.key_size;
    u8 *value_ptr = hm->values + i * hm->ctx.value_size;
    hm_put_assume_capacity(map, key_ptr, value_ptr);
  }
  hm->capacity = new_cap;
  free(hm->values);
  free(hm->keys);
  free(hm->isfree);
  hm->values = map->values;
  hm->keys = map->keys;
  hm->isfree = map->isfree;
  free(map);
}

kv_entry hm_get_or_put(hash_map *hm, const void *key) {
  grow_if_needed(hm);
  u32 indx = hm_get_index(hm, key);
  u8 *value_ptr = hm->values + hm->ctx.value_size * indx;
  u8 *key_ptr = hm->keys + hm->ctx.key_size * indx;
  if (!hm->isfree[indx]) {
    return (kv_entry){
        .found_existing = 1, .value_ptr = value_ptr, .key_ptr = key_ptr};
  }
  memcpy(key_ptr, key, hm->ctx.key_size);
  hm->isfree[indx] = 0;
  hm->size++;
  return (kv_entry){
      .found_existing = 0, .key_ptr = key_ptr, .value_ptr = value_ptr};
}

void hm_put(hash_map *hm, const void *key, const void *value) {
  kv_entry entry = hm_get_or_put(hm, key);
  memcpy(entry.value_ptr, value, hm->ctx.value_size);
}

void hm_deinit(hash_map *hm) {
  free(hm->keys);
  free(hm->values);
  free(hm->isfree);
  free(hm);
}

kv_iterator hm_iterator(hash_map *hm) {
  return (kv_iterator){.hm = hm,
                       .cntr = hm->size,
                       .indx = 0,
                       .key_ptr = NULL,
                       .value_ptr = NULL};
}

b8 get_next(kv_iterator *kvi) {
  if (kvi->cntr == 0)
    return 0;
  if (kvi->cntr < kvi->hm->size)
    kvi->indx++;
  while (kvi->hm->isfree[kvi->indx]) {
    kvi->indx++;
  }
  kvi->cntr--;
  kvi->value_ptr = kvi->hm->values + kvi->indx * kvi->hm->ctx.value_size;
  kvi->key_ptr = kvi->hm->keys + kvi->indx * kvi->hm->ctx.key_size;
  return 1;
}

#define STR8_LIT(s) ((string8){.str = (u8 *)(s), .size = strlen((s))})
#define STR8_FMT "%.*s"
#define STR8_UNWRAP(s) (int)(s).size, (char *)(s).str

b8 string8_eql(const hash_map_context ctx, const void *a, const void *b) {
  assert(ctx.key_size == sizeof(string8));
  string8 *sa = (string8 *)a;
  string8 *sb = (string8 *)b;
  if (sa->size != sb->size)
    return 0;
  for (u32 i = 0; i < sa->size; ++i) {
    if (sa->str[i] != sb->str[i])
      return 0;
  }
  return 1;
}

string8 to_string8(const char *s) {
  u64 size = strlen(s);
  u8 *str = (u8 *)malloc(strlen(s));
  memcpy(str, s, size);
  return (string8){.str = str, .size = size};
}

#endif
#endif
