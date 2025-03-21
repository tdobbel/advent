use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::HashMap;
use std::env;
use anyhow::Result;

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

    let mut found: bool;
    for line in reader.lines() {
        let line = line?;
        let mut end = line.len()-1;
        let start = line.chars().position(|c| c.is_ascii_digit()).unwrap();
        let x1 = line.chars().nth(start).unwrap().to_digit(10).unwrap();
        let mut x2 = x1; 
        found = false;
        for i in 1..start+1 {
            for (k,v) in numbers.iter() {
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
        while !line.chars().nth(end).unwrap().is_ascii_digit() {
            end -= 1;
        }
        let y1 = line.chars().nth(end).unwrap().to_digit(10).unwrap();
        let mut y2 = y1;
        let mut i = line.len()-2;
        found = false;
        while i > end && !found {
            for (k,v) in numbers.iter() {
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
