import sys
import numpy as np

class Mapping:

    def __init__(self) -> None:
        self.mapping = []

    def add_range(self, dst: int, src: int, rng: int) -> None:
        self.mapping.append((dst, src, rng)) 

    def __call__(self, seed: int) -> int:
        for dst, src, rng in self.mapping:
            if src <= seed < src + rng:
                return dst + seed - src
        return seed

def read_input() -> tuple[list[int], list[Mapping]]:
    maps = {}
    with open(sys.argv[1]) as f:
        line = f.readline().strip()
        seeds = list(map(int, line.split(':')[1].split()))
        f.readline()
        line = f.readline().strip()
        maps = []
        while line:
            mapping = Mapping()
            line = f.readline().strip()
            while len(line) > 0:
                mapping.add_range(*map(int, line.split()))
                line = f.readline().strip()
            maps.append(mapping)
            line = f.readline().strip()
    return seeds, maps

def apply_mapping(seed: int, maps: list[dict[int,int]]) -> int:
    res = seed
    for map in maps:
        res = map(res)
    return res

def map_interval(a: int, b: int, mapping: Mapping) -> list[tuple[int,int]]:
    range_map = {}
    res = []
    for dst, src, rng in mapping.mapping:
        range_map[(src,src+rng)] = (dst,dst+rng)
    intervals = list(range_map.keys())
    intervals.sort(key=lambda x: x[0])
    a_, _ = intervals[0]
    if a < a_:
        res.append((a, a_))
        a = a_ 
    for i, (start, end) in enumerate(intervals):
        dst_from, dst_to = range_map[(start, end)]
        if a >= end:
            continue
        a_ = max(a, start)
        b_ = min(b, end)
        res.append((dst_from + a_ -start, dst_from + b_ - start))
        a = b_ 
        if a >= b:
            break
    if a < b:
        res.append((a, b))
    return res

def apply_mapping_intervals(start: int, size: int) -> list[tuple[int,int]]:
    intervals = [(start, start + size)]
    for map in maps:
        new_intervals = []
        for (a, b) in intervals:
            new_intervals.extend(map_interval(a, b, map))
        intervals = new_intervals
    return intervals

if __name__ == "__main__":
    seeds, maps = read_input()
    part1 = min(map(lambda seed: apply_mapping(seed, maps), seeds))
    vmin = np.infty
    for i in range(0, len(seeds), 2):
        intervals = apply_mapping_intervals(seeds[i], seeds[i+1])
        vmin = min(vmin, min(map(lambda x: x[0], intervals)))
    part2 = vmin
    print(f"Part 1: {part1}")
    print(f"Part 2: {part2}")
