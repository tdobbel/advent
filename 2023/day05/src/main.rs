use anyhow::Result;
use std::env::args;
use std::fs::File;
use std::io::{BufRead, BufReader};

struct Map {
    ranges: Vec<(usize, usize, usize)>,
}

impl Map {
    pub fn new() -> Self {
        Self { ranges: Vec::new() }
    }

    pub fn add_range(&mut self, dst: usize, src: usize, range: usize) {
        self.ranges.push((dst, src, range));
    }

    pub fn apply(&self, seed: usize) -> usize {
        for (dst, src, range) in self.ranges.iter() {
            if seed >= *src && seed < *src + *range {
                return dst + seed - src;
            }
        }
        seed
    }

    pub fn apply_interval(&self, a: usize, b: usize) -> Vec<(usize, usize)> {
        let mut result = Vec::new();
        let mut a_ = a;
        for (dst, src, rng) in self.ranges.iter() {
            if *src > b {
                break;
            }
            if a_ >= *src + *rng {
                continue;
            }
            if a_ < *src {
                result.push((a_, *src));
                a_ = *src;
            }
            let start = a_.max(*src);
            let end = b.min(*src + *rng);
            result.push((*dst + start - *src, *dst + end - *src));
            a_ = end;
            if a_ >= b {
                break;
            }
        }
        if a_ < b {
            result.push((a_, b));
        }
        result
    }
}

fn apply_maps(maps: &[Map], seed: usize) -> usize {
    let mut result = seed;
    for map in maps.iter() {
        result = map.apply(result);
    }
    result
}

fn apply_maps_to_interval(maps: &[Map], src: usize, rng: usize) -> Vec<(usize, usize)> {
    let mut intervals = vec![(src, src + rng)];
    for map in maps.iter() {
        let mut next = Vec::new();
        for (a, b) in intervals.iter() {
            next.extend(&map.apply_interval(*a, *b));
        }
        intervals = next;
    }
    intervals
}

fn main() -> Result<()> {
    let filename = args().nth(1).expect("Missin input file");
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let mut first = true;
    let mut seeds: Vec<usize> = Vec::new();
    let mut maps: Vec<Map> = Vec::new();
    for line in reader.lines() {
        let line = line?;
        if first {
            seeds = line
                .split_whitespace()
                .skip(1)
                .map(|s| s.parse().unwrap())
                .collect();
            first = false;
            continue;
        }
        if line.is_empty() {
            continue;
        }
        if line.ends_with(':') {
            maps.push(Map::new());
        } else {
            let map_parts: Vec<usize> = line
                .split_whitespace()
                .map(|s| s.parse().unwrap())
                .collect();
            maps.last_mut()
                .unwrap()
                .add_range(map_parts[0], map_parts[1], map_parts[2]);
        }
    }
    // sort all ranges
    for map in maps.iter_mut() {
        map.ranges.sort_by_key(|k| k.1);
    }

    // part 1
    let mut sol1 = usize::MAX;
    for seed in seeds.iter() {
        sol1 = sol1.min(apply_maps(&maps, *seed));
    }
    println!("Part 1: {}", sol1);

    // part 2
    let mut sol2 = usize::MAX;
    for i in 0..seeds.len() / 2 {
        let intervals = apply_maps_to_interval(&maps, seeds[i * 2 + 0], seeds[i * 2 + 1]);
        sol2 = sol2.min(intervals.iter().map(|x| x.0).min().unwrap());
    }
    println!("Part 2 : {}", sol2);

    Ok(())
}
