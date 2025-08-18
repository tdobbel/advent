use anyhow::Result;
use regex::Regex;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn solve_system(xa: i64, ya: i64, xb: i64, yb: i64, x: i64, y: i64) -> Option<i64> {
    let det = xa * yb - ya * xb;
    if det == 0 {
        return None;
    }
    let na = (x * yb - y * xb) / det;
    let nb = (xa * y - ya * x) / det;
    if na < 0 || nb < 0 || na * xa + nb * xb != x || na * ya + nb * yb != y {
        return None;
    }
    Some(3 * na + nb)
}

fn main() -> Result<()> {
    let filename = env::args().nth(1).expect("Missing input file");
    let file = File::open(filename)?;
    let reader = BufReader::new(file);
    let mut xa = 0;
    let mut ya = 0;
    let mut xb = 0;
    let mut yb = 0;
    let pattern = Regex::new(r"([a-zA-z ]+): X[\+=]([0-9]+), Y[\+=]([0-9]+)")?;
    let mut part1 = 0;
    let mut part2 = 0;
    for line in reader.lines() {
        let line = line?;
        if line.is_empty() {
            continue;
        }
        let cap = pattern.captures(&line).unwrap();
        let key = cap.get(1).unwrap().as_str();
        let x_value = cap.get(2).unwrap().as_str().parse::<i64>()?;
        let y_value = cap.get(3).unwrap().as_str().parse::<i64>()?;
        match key {
            "Button A" => {
                xa = x_value;
                ya = y_value;
            }
            "Button B" => {
                xb = x_value;
                yb = y_value;
            }
            "Prize" => {
                if let Some(n) = solve_system(xa, ya, xb, yb, x_value, y_value) {
                    part1 += n;
                }
                if let Some(n) = solve_system(
                    xa,
                    ya,
                    xb,
                    yb,
                    x_value + 10000000000000,
                    y_value + 10000000000000,
                ) {
                    part2 += n;
                }
            }
            _ => eprintln!("Could not parse line '{}'", line),
        }
    }

    println!("Part 1: {}", part1);
    println!("Part 2: {}", part2);
    Ok(())
}
