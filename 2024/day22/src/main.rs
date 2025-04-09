use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn mix(x: u64, y: u64) -> u64 {
    x ^ y
}

fn prune(x: u64) -> u64 {
    x & ((1 << 24) - 1)
}

fn next_number(x: u64) -> u64 {
    let mut res;
    res = prune(mix(x << 6, x));
    res = prune(mix(res >> 5, res));
    res = prune(mix(res << 11, res));
    res
}

fn ones_digit(x: u64) -> i32 {
    (x % 10) as i32
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);

    let mut best = 0;
    let mut best_seq = Vec::<i32>::new();
    let mut total_prices = HashMap::<Vec<i32>, i32>::new();
    for line in reader.lines() {
        let line = line.unwrap();
        let mut x = line.parse::<u64>().unwrap();
        let mut prev = ones_digit(x);
        let mut price;
        let mut seq: Vec<i32> = Vec::with_capacity(4);
        let mut seq_price = HashMap::<Vec<i32>, i32>::new();
        for _ in 0..2000 {
            if seq.len() > 3 {
                seq.remove(0);
            }
            x = next_number(x);
            price = ones_digit(x);
            seq.push(price - prev);
            prev = price;
            if seq.len() < 4 {
                continue;
            }
            if price == 0 {
                continue;
            }
            let p = seq_price.entry(seq.clone()).or_insert(0);
            if *p > 0 {
                continue;
            }
            *p += price
        }
        for (seq, price) in seq_price.iter() {
            let p = total_prices.entry(seq.clone()).or_insert(0);
            *p += price;
            if *p > best {
                best = *p;
                best_seq.clone_from(seq);
            }
        }
    }
    println!("Best gain is {} with sequence {:?}", best, best_seq);
}
