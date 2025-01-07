use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::HashMap;
use std::env;
use anyhow::Result;

fn find_number(line: &str, numbers: &HashMap<&str, u32>, start: bool) -> Option<u32> {
    let n = line.len();
    for i in 0..line.len() {
        let c = if start {
            line.chars().nth(i).unwrap()
        } else {
            line.chars().nth_back(i).unwrap()
        };
        match c.to_digit(10) {
            Some(n) => return Some(n),
            None => {
                if i < 2 {
                    continue;
                }
                let slice_ = if start {
                    let indx_from = if i < 4 {0} else {i-4};
                    &line[indx_from..i+1]
                }else {
                    let indx_from = n-i-1;
                    let indx_to = if i < 4 {n} else {indx_from+5}; 
                    &line[indx_from..indx_to]
                };
                for (k,v) in numbers.iter() {
                    if slice_.contains(k) {
                        return Some(*v);
                    }
                }
            }
        }
    }
    None
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide a file name");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut numbers = HashMap::<&str, u32>::new();
    let mut total1 = 0;
    let mut total2 = 0;
    numbers.insert("one", 1);
    numbers.insert("two", 2);
    numbers.insert("three", 3);
    numbers.insert("four", 4);
    numbers.insert("five", 5);
    numbers.insert("six", 6);
    numbers.insert("seven", 7);
    numbers.insert("eight", 8);
    numbers.insert("nine", 9);
    for line in reader.lines() {
        let line = line?;
        let mut x = None;
        let mut y = None;
        for i in 0..line.len() {
            let c = line.chars().nth(i).unwrap();
            if let Some(n) = c.to_digit(10) { x = Some(n); }
            let c = line.chars().nth_back(i).unwrap();
            if let Some(n) = c.to_digit(10) { y = Some(n); }
            if x.is_some() && y.is_some() {
                break
            }
        }
        let x = x.unwrap();
        let y = y.unwrap();
        total1 += 10*x+y;
        let x = find_number(&line, &numbers, true).unwrap();
        let y = find_number(&line, &numbers, false).unwrap(); 
        total2 += 10*x+y;
    }
    println!("Part 1: {}", total1);
    println!("Part 2: {}", total2);
    Ok(())
}
