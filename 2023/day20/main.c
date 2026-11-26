#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define ARENA_IMPLEMENTATION
#include "arena.h"
#define STRING_IMPLEMENTATION
#include "string8.h"
#define VECTOR_IMPLEMENTATION
#include "vector.h"
#define HASHMAP_IMPLEMENTATION
#include "hash_map.h"

#define LOW 0
#define HIGH 1
#define N_PUSH 1000

typedef enum { Untyped, Broadcaster, FlipFlop, Conjunction } ModuleType;

typedef struct {
  ModuleType type;
  void *module;
  u32 n_dest;
  string8 destinations[8];
} Module;

typedef struct {
  b8 state; // 0 is off, 1 is on
} FlipFlopModule;

typedef struct {
  u32 n_input;
  string8 input_names[16];
  b8 memory[16];
} ConjunctionModule;

typedef struct {
  string8 src, dst;
  b8 pulse;
} Pulse;

#define BROADCASTER_STR STR8_LIT("broadcaster")
#define BUTTON_STR STR8_LIT("button")

void pulse_print(Pulse pulse);

b8 flip_flop_process_pulse(FlipFlopModule *ffm, Pulse input, b8 *output);
b8 conjunction_process_pulse(ConjunctionModule *cm, Pulse input, b8 *output);
void module_process_pulse(Module *mod, Pulse input, Queue *pulses);

void build_conjunction_memory(hash_map *modules);
void push_button(hash_map *modules, Queue *pulses, u64 counter[2]);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Missing input file\n");
    return EXIT_FAILURE;
  }

  mem_arena *gpa = arena_create(KiB(500));

  string8 file = {0};
  str_read_file(gpa, &file, argv[1]);

  hash_map *modules = STRING_HASHMAP(Module);

  vector *line_vec = VEC_CREATE(string8);
  vector *word_vec = VEC_CREATE(string8);

  str_split(line_vec, str_trim(file), STR8_LIT("\n"));
  string8 *lines = VEC_CAST(line_vec, string8);
  Module untyped = (Module){.type = Untyped};
  for (u32 i = 0; i < line_vec->size; ++i) {
    str_split_whitespace(word_vec, lines[i]);
    Module mod = {0};
    string8 *words = VEC_CAST(word_vec, string8);
    string8 name = {0};
    if (words[0].str[0] == '&') {
      mod.type = Conjunction;
      name = (string8){.str = words[0].str + 1, .size = words[0].size - 1};
      ConjunctionModule *cm = ALLOC_STRUCT(gpa, ConjunctionModule);
      cm->n_input = 0;
      mod.module = cm;
    } else if (words[0].str[0] == '%') {
      mod.type = FlipFlop;
      name = (string8){.str = words[0].str + 1, .size = words[0].size - 1};
      FlipFlopModule *ffm = ALLOC_STRUCT(gpa, FlipFlopModule);
      ffm->state = 0;
      mod.module = ffm;
    } else {
      mod.type = Broadcaster;
      mod.module = NULL;
      name = words[0];
    }
    for (u32 iw = 2; iw < word_vec->size; ++iw) {
      mod.destinations[mod.n_dest++] =
          str_remove_suffix(words[iw], STR8_LIT(","));
      if (!hm_contains_key(modules, &words[iw])) {
        hm_put(modules, &words[iw], &untyped);
      }
    }
    hm_put(modules, &name, &mod);
  }

  vector_deinit(line_vec);
  vector_deinit(word_vec);

  build_conjunction_memory(modules);
  Queue *pulses = QUEUE_CREATE(Pulse);
  u64 counter[2] = {0, 0};
  for (u32 i = 0; i < N_PUSH; ++i) {
    push_button(modules, pulses, counter);
  }
  printf("%lu, %lu -> %lu\n", counter[LOW], counter[HIGH],
         counter[LOW] * counter[HIGH]);

  queue_deinit(pulses);
  hm_deinit(modules);

  arena_destroy(gpa);

  return EXIT_SUCCESS;
}

b8 flip_flop_process_pulse(FlipFlopModule *ffm, Pulse input, b8 *output) {
  if (input.pulse == HIGH)
    return false;
  *output = ffm->state ? LOW : HIGH;
  ffm->state = !ffm->state;
  return true;
}

b8 conjunction_process_pulse(ConjunctionModule *cm, Pulse input, b8 *output) {
  b8 all_high = true;
  b8 found = false;
  for (u32 i = 0; i < cm->n_input; ++i) {
    if (str_equal(cm->input_names[i], input.src)) {
      cm->memory[i] = input.pulse;
      found = true;
    }
    if (cm->memory[i] == LOW)
      all_high = false;
    if (found && !all_high)
      break;
  }
  assert(found);
  *output = all_high ? LOW : HIGH;
  return true;
}

void build_conjunction_memory(hash_map *modules) {
  kv_iterator iter = hm_iterator(modules);
  while (get_next(&iter)) {
    string8 src = *(string8 *)iter.key_ptr;
    const Module *src_mod = (Module *)iter.value_ptr;
    for (u32 i = 0; i < src_mod->n_dest; ++i) {
      string8 dst = src_mod->destinations[i];
      Module *dst_mod = (Module *)hm_get_value(modules, &dst);
      if (dst_mod->type != Conjunction)
        continue;
      ConjunctionModule *cm = (ConjunctionModule *)dst_mod->module;
      cm->input_names[cm->n_input] = src;
      cm->memory[cm->n_input] = LOW;
      cm->n_input++;
    }
  }
}

void module_process_pulse(Module *mod, Pulse input, Queue *pulses) {
  b8 send_pulse = false;
  b8 out_pulse;
  switch (mod->type) {
  case Untyped: {
    send_pulse = false;
  } break;
  case Broadcaster: {
    send_pulse = true;
    out_pulse = input.pulse;
  } break;
  case FlipFlop:
    send_pulse = flip_flop_process_pulse((FlipFlopModule *)mod->module, input,
                                         &out_pulse);
    break;
  case Conjunction:
    send_pulse = conjunction_process_pulse((ConjunctionModule *)mod->module,
                                           input, &out_pulse);
    break;
  }
  if (!send_pulse)
    return;
  for (u32 iout = 0; iout < mod->n_dest; ++iout) {
    Pulse out = (Pulse){
        .src = input.dst, .dst = mod->destinations[iout], .pulse = out_pulse};
    queue_push(pulses, &out);
  }
}

void push_button(hash_map *modules, Queue *pulses, u64 counter[2]) {
  Pulse trigger =
      (Pulse){.src = BUTTON_STR, .dst = BROADCASTER_STR, .pulse = LOW};
  queue_reset(pulses);
  queue_push(pulses, &trigger);
  Pulse pulse = {0};
  while (queue_pop(pulses, &pulse)) {
    counter[pulse.pulse]++;
    // pulse_print(pulse);
    Module *mod = (Module *)hm_get_value(modules, &pulse.dst);
    module_process_pulse(mod, pulse, pulses);
  }
}

void pulse_print(Pulse pulse) {
  printf(STR8_FMT " -%s-> " STR8_FMT "\n", STR8_UNWRAP(pulse.src),
         pulse.pulse == LOW ? "low" : "high", STR8_UNWRAP(pulse.dst));
}
