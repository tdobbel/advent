use anyhow::Result;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

mod camel_cards;
use crate::camel_cards::*;

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("No input file provided");
    let file = File::open(&filename)?;
    let reader = BufReader::new(file);
    let mut hands: Vec<Hand> = Vec::new();
    for line in reader.lines() {
        let line = line?;
        let parts: Vec<&str> = line.split_whitespace().collect();
        let bid = parts[1].parse::<u32>().unwrap();
        let cards: [Card; 5] = parts[0]
            .chars()
            .map(|c| Card::from_char(c).unwrap())
            .collect::<Vec<Card>>()
            .try_into()
            .unwrap();
        hands.push(Hand::create(cards, bid as usize));
    }
    hands.sort_by(|a, b| compare_hand(a, b, false));
    println!("Part 1: {}", compute_score(&hands));
    hands.iter_mut().for_each(apply_joker);
    hands.sort_by(|a, b| compare_hand(a, b, true));
    println!("Part 2: {}", compute_score(&hands));
    Ok(())
}
