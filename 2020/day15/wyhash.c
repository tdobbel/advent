#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define uint128_t __uint128_t

typedef uint64_t u64;
typedef uint128_t u128;
typedef uint8_t u8;

const u64 secret[4] = {
    0xa0761d6478bd642f,
    0xe7037ed1a0b428db,
    0x8ebc6af09c88c6e3,
    0x589965cc75374cc3,
};

typedef struct {
  u64 a, b;
  u64 state[3];
  u64 total_len;

  u8 buf[48];
  u64 buf_len;
} Wyhash;

static inline void mum(u64 *a, u64 *b) {
  u128 r = *a;
  r *= *b;
  *a = (u64)r;
  *b = (u64)(r >> 64);
}

static inline u64 mix(u64 a_, u64 b_) {
  u64 a = a_;
  u64 b = b_;
  mum(&a, &b);
  return a ^ b;
}

static inline void wy_round(Wyhash *self, const u8 *input) {
  for (u64 i = 0; i < 3; ++i) {
    u64 a = *(u128 *)(input + 8 * (2 * i));
    u64 b = *(u128 *)(input + 8 * (2 * i + 1));
    self->state[i] = mix(a ^ secret[i + 1], b ^ self->state[i]);
  }
}

Wyhash wyhash_init(u64 seed) {
  Wyhash self = (Wyhash){.total_len = 0, .buf_len = 0};
  self.state[0] = seed ^ mix(seed ^ secret[0], secret[1]);
  self.state[1] = self.state[0];
  self.state[2] = self.state[0];
  return self;
}

static inline void wyhash_small_key(Wyhash *self, const u8 *input,
                                    u64 input_len) {
  assert(input_len <= 16);
  if (input_len >= 4) {
    u64 end = input_len - 4;
    u64 quarter = (input_len >> 3) << 2;
    self->a = (*(u64 *)input << 32) | *(u64 *)(input + quarter);
    self->b = (*(u64 *)(input + end) << 32) | *(u64 *)(input + end - quarter);
  } else if (input_len > 0) {
    self->a = ((u64)input[0] << 16) | ((u64)input[input_len >> 1] << 8) |
              (u64)input[input_len - 1];
    self->b = 0;
  } else {
    self->a = 0;
    self->b = 0;
  }
}

static inline void final0(Wyhash *self) {
  self->state[0] ^= self->state[1] ^ self->state[2];
}

static inline void final1(Wyhash *self, const u8 *input_lb, u64 input_len,
                          u64 start_pos) {
  assert(input_len >= 16);
  assert(input_len - start_pos <= 48);
  const u8 *input = input_lb + start_pos;
  u64 len = input_len - start_pos;

  u64 i = 0;
  while (i + 16 < len) {
    self->state[0] = mix((*(u64 *)(input + 1)) ^ secret[1],
                         (*(u64 *)(input + i + 8)) ^ self->state[0]);
    i += 16;
  }
  self->a = *(u64 *)(input_lb + input_len - 16);
  self->b = *(u64 *)(input_lb + input_len - 8);
}

static inline u64 final2(Wyhash *self) {
  self->a ^= secret[1];
  self->b ^= self->state[0];
  mum(&self->a, &self->b);
  return mix(self->a ^ secret[0] ^ self->total_len, self->b ^ secret[1]);
}

u64 wyhash(const u8 *input, u64 input_len, u64 seed) {
  Wyhash self = wyhash_init(seed);
  if (input_len <= 16) {
    wyhash_small_key(&self, input, input_len);
  } else {
    u64 i = 0;
    if (input_len >= 48) {
      while (i + 48 < input_len) {
        wy_round(&self, input);
        i += 48;
      }
      final0(&self);
    }
    final1(&self, input, input_len, i);
  }

  self.total_len = input_len;
  return final2(&self);
}

int main(void) {
  u64 hash = wyhash((u8 *)"message digest", 14, 3);
  printf("%lu - %lu\n", hash, 0x8619124089a3a16b);
  return 0;
}
