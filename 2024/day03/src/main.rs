use regex::Regex;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut part1 = 0;
    let mut part2 = 0;
    let mut active = true;
    let re = Regex::new(r"mul\((\d+),(\d+)\)").unwrap();
    for line in reader.lines() {
        let line = line.unwrap();
        let split: Vec<&str> = line.split("()").collect();
        for entry in split.iter() {
            for cap in re.captures_iter(entry) {
                let x = cap.get(1).unwrap().as_str().parse::<usize>().unwrap();
                let y = cap.get(2).unwrap().as_str().parse::<usize>().unwrap();
                let product = x * y;
                part1 += product;
                if active {
                    part2 += product;
                }
            }
            if entry.ends_with("don't") {
                active = false;
            } else if entry.ends_with("do") {
                active = true;
            }
        }
    }
    println!("Part 1: {}", part1);
    println!("Part 2: {}", part2);
}
