use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut total = 0;
    let mut counters = HashMap::<usize, usize>::new();
    for (i, line) in reader.lines().enumerate() {
        counters.entry(i).and_modify(|x| *x += 1).or_insert(1);
        let line = line.unwrap();
        let indx = line.find(':').unwrap();
        let line = &line[indx + 1..];
        let (left, right) = line.split_at(line.find('|').unwrap());
        let winning: Vec<i32> = left
            .split_whitespace()
            .map(|x| x.parse().unwrap())
            .collect();
        let cards: Vec<i32> = right[1..]
            .split_whitespace()
            .map(|x| x.parse().unwrap())
            .collect();
        let n_win = cards.iter().filter(|&x| winning.contains(x)).count();
        let incr = counters.get(&i).unwrap().clone();
        for j in 0..n_win {
            let k = i + j + 1;
            counters.entry(k).and_modify(|x| *x += incr).or_insert(incr);
        }
        if n_win > 0 {
            total += 1 << (n_win - 1);
        }
    }
    let n_cards = counters.iter().map(|(_, v)| v).sum::<usize>();
    println!("{}", total);
    println!("{} scratchcards", n_cards);
}
