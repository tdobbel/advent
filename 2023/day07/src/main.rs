use std::cmp::{max, Ordering};
use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn score_from_counter(cntr: &HashMap<char, u8>) -> u8 {
    let mut max_value = 0;
    for (_, count) in cntr.iter() {
        max_value = max(max_value, *count);
    }
    let result: u8;
    if max_value == 1 {
        result = 0;
    } else if max_value == 2 && cntr.len() == 4 {
        result = 1;
    } else if max_value == 2 && cntr.len() == 3 {
        result = 2;
    } else if max_value == 3 && cntr.len() == 3 {
        result = 3;
    } else if max_value == 3 && cntr.len() == 2 {
        result = 4;
    } else if max_value == 4 {
        result = 5;
    } else if max_value == 5 {
        result = 6;
    } else {
        panic!("Invalid hand");
    }
    result
}

fn get_hand_score(hand: &str, use_joker: bool) -> u8 {
    let mut cntr = HashMap::<char, u8>::new();
    let mut kmax: char = '0';
    let mut vmax: u8 = 0;
    let mut has_joker = false;
    for c in hand.chars() {
        let count = cntr.entry(c).or_insert(0);
        *count += 1;
        if c == 'J' {
            has_joker = true;
        } else if *count > vmax {
            vmax = *count;
            kmax = c;
        }
    }
    if !use_joker {
        return score_from_counter(&cntr);
    }
    if cntr.len() > 1 && has_joker {
        let n_joker = cntr.get(&'J').unwrap().clone();
        let count = cntr.entry(kmax).or_insert(0);
        *count += n_joker;
        cntr.remove(&'J');
    }
    return score_from_counter(&cntr);
}

fn card_to_value(card: char, use_joker: bool) -> u8 {
    if !use_joker {
        match card {
            '2' => 2,
            '3' => 3,
            '4' => 4,
            '5' => 5,
            '6' => 6,
            '7' => 7,
            '8' => 8,
            '9' => 9,
            'T' => 10,
            'J' => 11,
            'Q' => 12,
            'K' => 13,
            'A' => 14,
            _ => panic!("Invalid card"),
        }
    } else {
        match card {
            '2' => 2,
            '3' => 3,
            '4' => 4,
            '5' => 5,
            '6' => 6,
            '7' => 7,
            '8' => 8,
            '9' => 9,
            'T' => 10,
            'J' => 1,
            'Q' => 12,
            'K' => 13,
            'A' => 14,
            _ => panic!("Invalid card"),
        }
    }
}

fn compare_hands(hand1: &str, hand2: &str, use_joker: bool) -> Ordering {
    let score1 = get_hand_score(hand1, use_joker);
    let score2 = get_hand_score(hand2, use_joker);
    if score1 > score2 {
        return Ordering::Greater;
    } else if score1 < score2 {
        return Ordering::Less;
    }
    for i in 0..hand1.len() {
        let v1 = card_to_value(hand1.chars().nth(i).unwrap(), use_joker);
        let v2 = card_to_value(hand2.chars().nth(i).unwrap(), use_joker);
        if v1 > v2 {
            return Ordering::Greater;
        } else if v1 < v2 {
            return Ordering::Less;
        }
    }
    return Ordering::Equal;
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut hands_with_bids: Vec<(String, u32)> = Vec::new();
    for line in reader.lines() {
        let line = line.unwrap();
        let parts: Vec<String> = line.split_whitespace().map(|x| x.to_string()).collect();
        let bid = parts[1].parse::<u32>().unwrap();
        hands_with_bids.push((parts[0].clone(), bid));
    }
    hands_with_bids.sort_by(|a, b| compare_hands(&a.0, &b.0, false));
    let mut total1 = 0;
    for (i, (_, bid)) in hands_with_bids.iter().enumerate() {
        total1 += (i as u32 + 1) * bid;
    }
    println!("{}", total1);
    hands_with_bids.sort_by(|a, b| compare_hands(&a.0, &b.0, true));
    let mut total2 = 0;
    for (i, (_, bid)) in hands_with_bids.iter().enumerate() {
        total2 += (i as u32 + 1) * bid;
    }
    println!("{}", total2);
}
