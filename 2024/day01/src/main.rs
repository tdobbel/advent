use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut left: Vec<usize> = Vec::new();
    let mut right: Vec<usize> = Vec::new();
    let mut linevalues: Vec<usize>;
    for line in reader.lines() {
        let line = line.unwrap();
        linevalues = line
            .split_whitespace()
            .map(|x| x.parse::<usize>().unwrap())
            .collect();
        left.push(linevalues[0]);
        right.push(linevalues[1]);
    }
    left.sort();
    right.sort();
    let mut part1 = 0;
    let mut part2 = 0;
    for (i, x) in left.iter().enumerate() {
        part1 += x.abs_diff(right[i]); 
        part2 +=  x * right.iter().filter(|&y| y == x).count();
    }
    println!("Part 1: {}", part1);
    println!("Part 2: {}", part2);
}
