use anyhow::Result;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

pub fn is_fresh(ingredient: u64, fresh_rng: &[(u64, u64)]) -> bool {
    for (lo, hi) in fresh_rng.iter() {
        if ingredient >= *lo && ingredient <= *hi {
            return true;
        }
        if ingredient < *lo {
            break;
        }
    }
    false
}

pub fn simplify_overlaps(fresh_rng: &[(u64, u64)]) -> Vec<(u64, u64)> {
    let mut simpl = Vec::new();
    let mut i = 0;
    while i < fresh_rng.len() {
        let (lo, mut hi) = fresh_rng[i];
        let mut j = i + 1;
        while j < fresh_rng.len() {
            let (start, stop) = fresh_rng[j];
            if start > hi {
                break;
            }
            hi = hi.max(stop);
            j += 1;
        }
        simpl.push((lo, hi));
        i = j;
    }
    simpl
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut fresh_ranges: Vec<(u64, u64)> = Vec::new();
    let mut lines = reader.lines();
    while let Some(line) = lines.next() {
        let line = line?;
        if line.is_empty() {
            break;
        }
        let (lo, hi) = line.split_once("-").unwrap();
        fresh_ranges.push((lo.parse().unwrap(), hi.parse().unwrap()));
    }
    fresh_ranges.sort_by(|a, b| a.0.cmp(&b.0));
    let fresh_ranges = simplify_overlaps(&fresh_ranges);
    let mut part1 = 0;
    let part2 = fresh_ranges
        .iter()
        .map(|(lo, hi)| *hi - *lo + 1)
        .sum::<u64>();
    while let Some(line) = lines.next() {
        let line = line?;
        let ingredient = line.parse().unwrap();
        if is_fresh(ingredient, &fresh_ranges) {
            part1 += 1;
        }
    }
    println!("Part 1: {part1}");
    println!("Part 2: {part2}");
    Ok(())
}
