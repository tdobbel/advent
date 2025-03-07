use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).expect("Missing input file");
    let reader = BufReader::new(file);
    let mut value = 0;
    let mut categories: Vec<usize> = Vec::new();
    for line in reader.lines() {
        let line = line.unwrap();
        if line.is_empty() {
            categories.push(value);
            value = 0;
        } else {
            value += line.parse::<usize>().unwrap();
        }
    }
    if value > 0 {
        categories.push(value);
    }
    categories.sort();
    let n = categories.len();
    println!("Part 1: {}", categories.last().unwrap());
    println!("Part 2: {}", &categories[n-3..].iter().sum::<usize>());
}
