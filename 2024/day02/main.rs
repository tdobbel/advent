use std::fs::File;
use std::io::{BufRead, BufReader};
use std::env;

fn issafe(numbers: &Vec<i32>) -> bool {
    let increasing = numbers[1] > numbers[0];
    for i in 1..numbers.len() {
        let mut diff = numbers[i] - numbers[i-1];
        if !increasing {
            diff = -diff;
        }
        if diff < 1 || diff > 3 {
            return false;
        }
    }
    true
}

fn issafe_dampened(numbers: &Vec<i32>) -> bool {
    if issafe(numbers) {
        return true;
    }
    for i in 0..numbers.len() {
        let smaller = numbers.iter().enumerate().filter(|&(j, _)| j != i).map(|(_, x)| *x).collect::<Vec<i32>>();
        if issafe(&smaller) {
            return true;
        }
    }
    false
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let file = File::open(args[1].clone()).unwrap();
    let reader = BufReader::new(file);
    let mut total1 = 0;
    let mut total2 = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        let numbers = line.split_whitespace().map(|x| x.parse::<i32>().unwrap()).collect::<Vec<i32>>();
        if issafe(&numbers) {
            total1 += 1;
        }
        if issafe_dampened(&numbers) {
            total2 += 1;
        }
    }
    println!("Part 1: {}", total1);
    println!("Part 2: {}", total2);
}
