use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::HashMap;
use std::cmp::min;

fn blink(stone: i32, new_stones: &mut Vec<i32>) {
    let stone_as_str = stone.to_string();
    if stone == 0 {
        new_stones.push(1);
    }
    else if stone_as_str.len() % 2 == 0 {
        let mid = stone_as_str.len() / 2;
        new_stones.push(stone_as_str[..mid].parse::<i32>().unwrap());
        new_stones.push(stone_as_str[mid..].parse::<i32>().unwrap());
    }
    else {
        new_stones.push(stone*2024);
    }
}

fn precompute_blink(seed: i32, n: i32) -> Vec<Vec<(i32,i32)>> {
    let mut states = Vec::<Vec<(i32,i32)>>::new();
    let mut prev = vec![(1,seed)];
    for i in 0..n {
        let mut next = Vec::<(i32,i32)>::new();
        for (factor,stone) in prev.into_iter() {
            let mut blinked = Vec::<i32>::new();
            blink(stone, &mut blinked);
            for v in blinked.iter() {
                next.push((factor,*v));
            }
        }
        let mut counter = HashMap::<i32,i32>::new();
        for (factor,value) in next.iter() {
            let count = counter.entry(*value).or_insert(0);
            *count += factor;
        }
        let new_state: Vec<(i32,i32)> = counter.iter().map(|(k,v)| (*v,*k)).collect();
        states.push(new_state.clone());
        prev = new_state;
    }
    states
}

fn blink_recur(stone: u64, rules: &HashMap<u64,Vec<Vec<(i32,i32)>>>, accum: usize) -> u64 {
    if accum == 0 {
        return 1
    }
    let rule = rules.get(&stone);
    if rule.is_some() {
        let rule = rule.unwrap();
        let mut result: u64 = 0;
        let n = min(accum, rule.len());
        for (f,v) in rule[n-1].iter() {
            result += (*f as u64) * blink_recur(*v as u64, rules, accum-n);
        }
        return result;

    }
    let stone_as_str = stone.to_string();
    if stone == 0 {
        return blink_recur(1, rules, accum - 1);
    }
    else if stone_as_str.len() % 2 == 0 {
        let mid = stone_as_str.len() / 2;
        let lhs = stone_as_str[..mid].parse::<u64>().unwrap();
        let rhs = stone_as_str[mid..].parse::<u64>().unwrap();
        return blink_recur(lhs, rules, accum - 1) + blink_recur(rhs, rules, accum - 1);
    }
    else {
        return blink_recur(stone * 2024 ,rules, accum - 1);
    }
}

fn main() {
    let mut rules = HashMap::<u64, Vec<Vec<(i32,i32)>>>::new();
    let n = 25;
    for i in 0..10 {
        let rule = precompute_blink(i,n);
        rules.insert(i as u64,rule.clone());
        for (_,v) in rule[(n-1) as usize].iter() {
            rules.entry(*v as u64).or_insert(precompute_blink(*v,n).clone());
        }
    }
    let file = File::open("input").unwrap();
    let reader = BufReader::new(file);
    let line = reader.lines().next().unwrap().unwrap();
    let n_blink = 75;
    let mut total = 0;
    for (i,c) in line.split_whitespace().enumerate() {
        println!("stone {}", i);
        let stone = c.parse::<u64>().unwrap();
        total += blink_recur(stone, &rules, n_blink);
    }
    println!("There are {} stones", total);
}
