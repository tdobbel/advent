use anyhow::Result;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

pub fn process_column_v1(numbers: &[String], op: char, start: usize, end: usize) -> u64 {
    let col = numbers
        .iter()
        .map(|line| line[start..end].trim().parse::<u64>().unwrap());
    if op == '+' { col.sum() } else { col.product() }
}

pub fn process_column_v2(numbers: &[String], op: char, start: usize, end: usize) -> u64 {
    let n = end - start;
    let mut col: Vec<u64> = vec![0; n];
    for line in numbers.iter() {
        for (i, c) in line[start..end].chars().enumerate() {
            if c == ' ' {
                continue;
            }
            col[i] = col[i] * 10 + c.to_digit(10).unwrap() as u64;
        }
    }
    if op == '+' {
        col.iter().sum()
    } else {
        col.iter().product()
    }
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut numbers: Vec<String> = Vec::new();
    let mut lines = reader.lines().peekable();
    let mut ops: Vec<char> = Vec::new();
    let mut col_start: Vec<usize> = Vec::new();

    while let Some(line) = lines.next() {
        let line = line?;
        if lines.peek().is_some() {
            numbers.push(line.to_string());
            continue;
        }
        for (i, c) in line.chars().enumerate() {
            if c == ' ' {
                continue;
            }
            ops.push(c);
            col_start.push(i);
        }
        col_start.push(line.len() + 1);
    }

    let mut part1: u64 = 0;
    let mut part2: u64 = 0;

    for (i, op) in ops.iter().enumerate() {
        part1 += process_column_v1(&numbers, *op, col_start[i], col_start[i + 1] - 1);
        part2 += process_column_v2(&numbers, *op, col_start[i], col_start[i + 1] - 1);
    }

    println!("Part 1: {part1}");
    println!("Part 2: {part2}");

    Ok(())
}
