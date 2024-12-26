use std::fs::File;
use std::io::{BufRead, BufReader};
use std::env;


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

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut total = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        let x = line.parse::<u64>().unwrap();
        let mut y = x;
        for _ in 0..2000 {
            y = next_number(y);
        }
        total += y;
    }
    println!("{}", total);
}
