use anyhow::Result;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut x: u32 = 0;
    let mut y: u32 = 0;
    let mut aim: u32 = 0;
    for line in reader.lines() {
        let line = line?;
        let splitted: Vec<&str> = line.split_whitespace().collect();
        let n: u32 = splitted[1].parse()?;
        match splitted[0] {
            "forward" => {
                x += n;
                y += aim * n;
            }
            "up" => aim -= n,
            "down" => aim += n,
            _ => panic!(),
        }
    }
    println!("Part 1: {}", x * aim);
    println!("Part 2: {}", x * y);

    Ok(())
}
