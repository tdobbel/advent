use std::fs::File;
use std::io::{BufRead, BufReader};

const PART2: bool = false;

#[derive(PartialEq)]
enum Operation{
    Add,
    Multiply,
    Concatenate,
}

impl Operation {
    const VALUES: [Operation; 3] = [Operation::Add, Operation::Multiply, Operation::Concatenate];
}

fn concat_numbers(x: u64, y: u64) -> u64 {
    let mut factor = 1;
    let mut z = y;
    while z > 0 {
        factor *= 10;
        z /= 10;
    }
    return x*factor + y;
}

fn check_line(total: u64, numbers: &Vec<u64>, index: usize, op: &Operation, accum: u64) -> u64 {
    if index == numbers.len() {
        if accum == total {
            return 1;
        } else {
            return 0;
        }
    }
    let result = match op {
        Operation::Add => accum + numbers[index],
        Operation::Multiply => accum * numbers[index],
        Operation::Concatenate => concat_numbers(accum, numbers[index]),
    };
    if result > total {
        return 0;
    }
    let mut next = 0;
    for next_op in Operation::VALUES.iter() {
        if !PART2 && next_op == &Operation::Concatenate {
            continue;
        }
        next += check_line(total, numbers, index+1, next_op, result);
    }
    next
}

fn main() {
    let file = File::open("input").unwrap();
    let reader = BufReader::new(file);
    let mut result = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        let linedata = line.split(":").collect::<Vec<&str>>();
        let total = linedata[0].parse::<u64>().unwrap();
        let numbers = linedata[1].split_whitespace().map(|x| x.parse::<u64>().unwrap()).collect::<Vec<u64>>();
        let mut nok = 0;
        for op in Operation::VALUES.iter() {
            if !PART2 && op == &Operation::Concatenate {
                continue;
            }
            nok += check_line(total, &numbers, 1, op, numbers[0]);
        }
        if nok > 0 {
            result += total;
        }
    }
    println!("Result: {}", result);
}
