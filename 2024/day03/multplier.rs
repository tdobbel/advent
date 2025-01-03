use std::fs::File;
use std::io::{BufRead, BufReader};

fn parse_mul(statement: &str) -> i32 {
    if statement.len() < 8 {
        return 0;
    }
    for i in 4..statement.len() {
        if &statement[i-4..i] == "mul(" {
            let args = &statement[i..statement.len()-1].split(',').collect::<Vec<&str>>();
            if args.len() != 2 {
                continue;
            }
            let mut argvalues = Vec::<i32>::new();
            for arg in args.iter() {
                let value = match arg.parse::<i32>() {
                    Ok(v) => v,
                    Err(_) => continue,
                };
                argvalues.push(value);
            }
            if argvalues.len() == 2 {
                return argvalues[0] * argvalues[1];
            }
        }
    }
    return 0;
}

fn main() {
    let file = File::open("input").unwrap();
    let reader = BufReader::new(file);
    let mut enabled = true;
    let mut index;
    let mut total1 = 0;
    let mut total2 = 0;
    for line in reader.lines() {
        index = 0;
        let line = line.unwrap();
        for (x,c) in line.chars().enumerate() {
            if c != ')' {
                continue
            }
            let substring = &line[index..x+1];
            if substring.contains("do()") {
                enabled = true;
            }
            else if substring.contains("don't()") {
                enabled = false;
            }
            else {
                let value = parse_mul(&substring);
                if enabled {
                    total2 += value;
                }
                total1 += value;
            }
            index = x+1;
        }
    }
    println!("Total part 1: {}", total1);
    println!("Total part 2: {}", total2);
}
