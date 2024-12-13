use std::fs::File;
use std::io::{BufRead, BufReader};

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
    let file = File::open("input").unwrap();
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
