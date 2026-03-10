use anyhow::Result;
use regex::Regex;
use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

pub fn valid_password(letter: char, min_count: usize, max_count: usize, password: &str) -> bool {
    let mut counter: HashMap<char, usize> = HashMap::new();
    for c in password.chars() {
        let cnt = counter.entry(c).or_insert(0);
        *cnt += 1;
    }
    let cnt = counter.entry(letter).or_insert(0);
    *cnt >= min_count && *cnt <= max_count
}

pub fn valid_password2(letter: char, index1: usize, index2: usize, password: &str) -> bool {
    let c1 = password.chars().nth(index1).unwrap();
    let c2 = password.chars().nth(index2).unwrap();
    (c1 == letter) ^ (c2 == letter)
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let pattern = Regex::new(r"(\d+)-(\d+) ([a-z]): ([a-z]+)")?;
    let mut part1 = 0;
    let mut part2 = 0;
    for line in reader.lines() {
        let line = line?;
        let caps = pattern.captures(&line).unwrap();
        let min_count: usize = caps[1].parse()?;
        let max_count: usize = caps[2].parse()?;
        let letter = caps[3].chars().next().unwrap();
        if valid_password(letter, min_count, max_count, &caps[4]) {
            part1 += 1;
        }
        if valid_password2(
            letter,
            min_count.checked_sub(1).unwrap(),
            max_count.checked_sub(1).unwrap(),
            &caps[4],
        ) {
            part2 += 1;
        }
    }
    println!("Part1: {part1} passwords are valid");
    println!("Part2: {part2} passwords are valid");
    Ok(())
}
