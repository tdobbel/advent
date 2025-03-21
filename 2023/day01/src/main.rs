use anyhow::Result;
use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide a file name");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut total1 = 0;
    let mut total2 = 0;
    let numbers: HashMap<&str, u32> = HashMap::from([
        ("one", 1),
        ("two", 2),
        ("three", 3),
        ("four", 4),
        ("five", 5),
        ("six", 6),
        ("seven", 7),
        ("eight", 8),
        ("nine", 9),
    ]);
    let mut found: bool;
    for line in reader.lines() {
        let line = line?;
        let start = line.chars().position(|c| c.is_ascii_digit()).unwrap();
        let x1 = line.chars().nth(start).unwrap().to_digit(10).unwrap();
        let mut x2 = x1;
        found = false;
        for i in 1..start + 1 {
            for (k, v) in numbers.iter() {
                if line[..i].contains(k) {
                    x2 = *v;
                    found = true;
                    break;
                }
            }
            if found {
                break;
            }
        }
        let end = line.len() - line.chars().rev().position(|c| c.is_ascii_digit()).unwrap() - 1;
        let y1 = line.chars().nth(end).unwrap().to_digit(10).unwrap();
        let mut y2 = y1;
        let mut i = line.len() - 2;
        found = false;
        while i > end && !found {
            for (k, v) in numbers.iter() {
                if line[i..].contains(k) {
                    y2 = *v;
                    found = true;
                    break;
                }
            }
            i -= 1;
        }

        total1 += 10 * x1 + y1;
        total2 += 10 * x2 + y2;
    }
    println!("Part 1: {}", total1);
    println!("Part 2: {}", total2);
    Ok(())
}
