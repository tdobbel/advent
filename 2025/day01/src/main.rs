use anyhow::{Result, anyhow};
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut dial: u32 = 50;
    let mut part1 = 0;
    let mut part2 = 0;
    for line in reader.lines() {
        let line = line?;
        let dir = line.chars().nth(0).unwrap();
        let mut n_step: u32 = line[1..].parse().unwrap();
        part2 += n_step / 100;
        n_step = n_step % 100;
        match dir {
            'L' => {
                dial = match dial.checked_sub(n_step) {
                    Some(n) => n,
                    None => {
                        if dial > 0 {
                            part2 += 1;
                        }
                        dial + 100 - n_step
                    }
                };
            }
            'R' => {
                dial += n_step;
                if dial > 100 {
                    part2 += 1;
                }
                dial = dial % 100;
            }
            _ => return Err(anyhow!("Invalid line format")),
        }
        if dial == 0 {
            part1 += 1;
            part2 += 1;
        }
    }
    println!("Part 1: {part1}");
    println!("Part 2: {part2}");
    Ok(())
}
