use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn extrapolate(numbers: &[i32], part1: &mut i32, part2: &mut i32) {
    let mut stop = false;
    let mut array = numbers.to_owned();
    let mut past: i32 = 0;
    let mut future: i32 = 0;
    let mut factor = 1;
    while !stop {
        future += array.last().unwrap();
        past += factor * array.first().unwrap();
        array = (0..array.len()-1).map(|i| array[i+1] - array[i]).collect();
        stop = array.iter().all(|x| *x == 0);
        factor *= -1;
    }
    *part1 += future;
    *part2 += past;
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut total1 = 0;
    let mut total2 = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        let numbers: Vec<i32> = line.split_whitespace()
            .map(|n| n.parse::<i32>().unwrap())
            .collect();
        extrapolate(&numbers, &mut total1, &mut total2);
    }
    println!("Part 1: {}", total1);
    println!("Part 2: {}", total2);
}
