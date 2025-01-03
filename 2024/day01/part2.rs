use std::fs::read_to_string;
use std::collections::HashMap;

fn main() {
    let mut col1: Vec<i32> = Vec::new();
    let mut counter = HashMap::<i32,i32>::new();

    for line in read_to_string("input").unwrap().lines() {
        let mut parts = line.split_whitespace();
        col1.push(parts.next().unwrap().parse().unwrap());
        let num = parts.next().unwrap().parse().unwrap();
        let count = counter.entry(num).or_insert(0);
        *count += 1;
    }
    let mut total = 0;
    for i in 0..col1.len() {
        let count = counter.entry(col1[i]).or_insert(0).clone();
        total += col1[i]*count;
    }
    println!("{}", total);
}
