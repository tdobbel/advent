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

fn decode_seq(seq: u32) -> [i8; 4] {
    let mut res = [0i8; 4];
    let mut seq = seq;
    for i in 0..4 {
        let num = seq & 0x1f;
        res[3 - i] = num as i8 - 9;
        seq = seq >> 5;
    }
    res
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);

    let mut best = 0;
    let mut best_seq: u32 = 0;
    let mut total_prices = HashMap::<u32, i32>::new();
    let cut = 1 << 20;
    let mut part1 = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        let mut secret = line.parse::<u64>().unwrap();
        let mut prev = ones_digit(secret);
        let mut price;
        let mut seq_price = HashMap::<u32, i32>::new();
        let mut seq: u32 = 0;
        for i in 0..2000 {
            secret = next_number(secret);
            price = ones_digit(secret);
            seq = (seq << 5) + ((price - prev + 9) as u32) & 0xfffff;
            seq = seq % cut;
            prev = price;
            if i < 3 || price == 0 {
                continue;
            }
            let p = seq_price.entry(seq).or_insert(0);
            if *p > 0 {
                continue;
            }
            *p += price
        }
        part1 += secret;
        for (seq, price) in seq_price.iter() {
            let p = total_prices.entry(*seq).or_insert(0);
            *p += price;
            if *p > best {
                best = *p;
                best_seq.clone_from(seq);
            }
        }
    }
    println!("Part 1: {}", part1);
    println!("Part 2: {} with sequence {:?}", best, decode_seq(best_seq));
}
