use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

enum Op {
    Add,
    Multiply,
    Concatenate,
}

impl Op {
    const ALL: [Op; 3] = [Op::Add, Op::Multiply, Op::Concatenate];
}

fn apply(op: &Op, a: u64, b: u64) -> u64 {
    match op {
        Op::Add => a + b,
        Op::Multiply => a * b,
        Op::Concatenate => {
            let mut c = b;
            let mut result = a;
            while c > 0 {
                result *= 10;
                c /= 10;
            }
            result + b
        },
    }
}

fn ispossible(total: u64, numbers: &[u64], index: usize, nop: usize, accum: u64) -> bool {
    if accum > total {
        return false;
    }
    if index == numbers.len() {
        return accum == total;
    }
    let mut result = false;
    for iop in 0..nop {
        let op = &Op::ALL[iop];
        let new_accum = apply(op, accum, numbers[index]);
        result |= ispossible(total, numbers, index + 1, nop, new_accum);
    }
    result
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut part1: u64 = 0;
    let mut part2: u64 = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        let n = line.chars().position(|c| c == ':').unwrap();
        let total = line[..n].parse::<u64>().unwrap();
        let numbers: Vec<u64> = line[n+1..]
            .split_whitespace()
            .map(|x| x.parse::<u64>().unwrap())
            .collect();
        if ispossible(total, &numbers, 1, 2, numbers[0]) {
            part1 += total;
        }
        if ispossible(total, &numbers, 1, 3, numbers[0]) {
            part2 += total;
        }
    }
    println!("Part 1: {}", part1);
    println!("Part 2: {}", part2);
}
