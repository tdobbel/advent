use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn extrapolate(numbers: &Vec<i32>, forward: bool) -> i32 {
    let mut stop = false;
    let mut array = numbers.clone();
    let mut accum = Vec::<Vec<i32>>::new();
    accum.push(array.clone());
    while !stop {
        let new_array: Vec<i32> = (0..array.len()-1).map(|i| array[i+1] - array[i]).collect();
        accum.push(new_array.clone());
        array = new_array;
        stop = array.iter().all(|x| *x == 0);
    }
    accum.reverse();
    let mut pred = 0;
    for a in accum.iter() {
        if forward { 
            match a.last() {
                Some(v) => pred = pred + v,
                None => panic!("Empty array"),
            };
        } else {
            match a.first() {
                Some(v) => pred = v - pred,
                None => panic!("Empty array"),
            };
        }
    }
    pred
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut series = Vec::<Vec<i32>>::new();
    for line in reader.lines() {
        let line = line.unwrap();
        let numbers: Vec<i32> = line.split_whitespace()
            .map(|n| n.parse::<i32>().unwrap())
            .collect();
        series.push(numbers);
    }
    let future = series.iter().map(|x| extrapolate(x,true)).sum::<i32>();
    let past = series.iter().map(|x| extrapolate(x,false)).sum::<i32>();
    println!("Part 1: {}", future);
    println!("Part 2: {}", past);
}
