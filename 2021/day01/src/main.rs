use anyhow::Result;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

const N: usize = 3;

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut prev: u32 = 0;
    let mut part1 = 0;
    let mut part2 = 0;
    let mut sum1: u32;
    let mut sum2: u32 = 0;
    let mut values: [u32; N] = Default::default();
    for (iln, line) in reader.lines().enumerate() {
        let line = line?;
        let depth: u32 = line.parse()?;
        if iln > 0 && depth > prev {
            part1 += 1;
        }
        prev = depth;
        values.iter_mut().enumerate().for_each(|(i, v)| {
            if i <= iln {
                *v += depth
            }
        });
        if iln < N - 1 {
            continue;
        }
        sum1 = sum2;
        sum2 = values[0];
        for i in 0..N - 1 {
            values[i] = values[i + 1];
        }
        values[N - 1] = 0;
        if iln >= N && sum2 > sum1 {
            part2 += 1;
        }
    }
    println!("Part 1: {part1}");
    println!("Part 2: {part2}");
    Ok(())
}
