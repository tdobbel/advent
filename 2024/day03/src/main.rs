use regex::Regex;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn exec_line(line: &str) -> usize {
    let mut result = 0;
    let re = Regex::new(r"mul\((\d+),(\d+)\)").unwrap();
    for cap in re.captures_iter(line) {
        let x = cap.get(1).unwrap().as_str().parse::<usize>().unwrap();
        let y = cap.get(2).unwrap().as_str().parse::<usize>().unwrap();
        result += x * y;
    }
    result
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut part1 = 0;
    let mut part2 = 0;
    let mut active = true;
    for line in reader.lines() {
        let line = line.unwrap();
        let split: Vec<&str> = line.split("()").collect();
        for entry in split.iter() {
            if active {
                part2 += exec_line(entry);
            }
            if entry.ends_with("don't") {
                active = false;
            } else if entry.ends_with("do") {
                active = true;
            }
        }
        part1 += exec_line(&line);
    }
    println!("Part 1: {}", part1);
    println!("Part 2: {}", part2);
}
