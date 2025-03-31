use anyhow::Result;
use std::cmp::Ordering;
use std::collections::HashMap;
use std::env::args;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn is_correct(numbers: &[u32], greater: &HashMap<u32, Vec<u32>>) -> bool {
    for i in 0..numbers.len() - 1 {
        let lower = match greater.get(&numbers[i]) {
            Some(x) => x,
            None => continue,
        };
        for j in i + 1..numbers.len() {
            if lower.contains(&numbers[j]) {
                return false;
            }
        }
    }
    true
}

fn main() -> Result<()> {
    let filename = args().nth(1).expect("Missing input file");
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let mut greater: HashMap<u32, Vec<u32>> = HashMap::new();
    let mut part1 = 0;
    let mut part2 = 0;
    for line in reader.lines() {
        let line = line?;
        if line.is_empty() {
            continue;
        }
        if line.contains("|") {
            let lineparts: Vec<u32> = line.split("|").map(|x| x.parse().unwrap()).collect();
            greater
                .entry(lineparts[1])
                .or_insert(vec![])
                .push(lineparts[0]);
        } else {
            let mut numbers: Vec<u32> = line.split(",").map(|x| x.parse().unwrap()).collect();
            if is_correct(&numbers, &greater) {
                part1 += numbers[numbers.len() / 2];
            } else {
                numbers.sort_by(|a, b| {
                    if let Some(x) = greater.get(a) {
                        if x.contains(b) {
                            return Ordering::Greater;
                        }
                    }
                    if let Some(x) = greater.get(b) {
                        if x.contains(a) {
                            return Ordering::Less;
                        }
                    }
                    Ordering::Equal
                });
                part2 += numbers[numbers.len() / 2];
            }
        }
    }

    println!("Part 1: {}", part1);
    println!("Part 2: {}", part2);

    Ok(())
}
