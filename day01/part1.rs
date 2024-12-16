use std::fs::read_to_string;

fn main() {
    let mut col1: Vec<i32> = Vec::new();
    let mut col2: Vec<i32> = Vec::new();

    for line in read_to_string("input").unwrap().lines() {
        let mut parts = line.split_whitespace();
        col1.push(parts.next().unwrap().parse().unwrap());
        col2.push(parts.next().unwrap().parse().unwrap());
    }
    col1.sort();
    col2.sort();
    let mut total = 0;
    for i in 0..col1.len() {
        total += (col1[i] - col2[i]).abs();
    }
    println!("{}", total);
}
