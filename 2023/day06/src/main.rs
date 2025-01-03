use std::{env, u64};
use std::fs::File;
use std::io::{BufRead, BufReader};

fn n_win(time: i32, distance: i32) -> usize {
    (1..time).filter(|&x| (time-x)*x >distance).count()
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut times: Vec<i32> = vec![];
    let mut distances: Vec<i32> = vec![];
    for (i, line) in reader.lines().enumerate() {
        let line = line.unwrap();
        let index = line.find(":").unwrap();
        let values: Vec<i32> = line[index+1..].split_whitespace()
            .map(|x| x.parse::<i32>().unwrap())
            .collect();
        if i == 0 {
            times = values;
        }
        else {
            distances = values;
        }
    }
    let total = (0..times.len()).map(|i| n_win(times[i], distances[i])).product::<usize>();
    let time = times
        .iter()
        .map(|x| x.to_string())
        .collect::<Vec<String>>()
        .join("")
        .parse::<f64>()
        .unwrap();
    let distance = distances
        .iter()
        .map(|x| x.to_string())
        .collect::<Vec<String>>()
        .join("")
        .parse::<f64>()
        .unwrap();
    let rho = f64::sqrt(time*time - 4.0*distance); 
    let x1 = f64::ceil((time - rho) / 2.0) as u64;
    let x2 = f64::floor((time + rho) / 2.0) as u64;
    println!("Part 1: {}", total);
    println!("Part 2: {}", x2-x1+1);
}

