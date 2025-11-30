use anyhow::{Result, anyhow};
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn get_rating(report: &mut [String], flag_oxygen: bool) -> Result<u32> {
    let mut slice = report;
    let num_size = slice[0].len();
    for i in 0..num_size {
        slice.sort_by(|a, b| {
            let ai = a.chars().nth(i).unwrap();
            let bi = b.chars().nth(i).unwrap();
            ai.cmp(&bi)
        });
        let n_zeros = slice
            .iter()
            .filter(|bits| bits.chars().nth(i).unwrap() == '0')
            .count();
        let n_ones = slice.len() - n_zeros;
        if flag_oxygen ^ (n_zeros > n_ones) {
            slice = &mut slice[n_zeros..];
        } else {
            slice = &mut slice[..n_zeros];
        }
        if slice.len() == 1 {
            let n = u32::from_str_radix(&slice[0], 2).unwrap();
            return Ok(n);
        }
    }
    Err(anyhow!("This should not be happening!"))
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args)?;
    let reader = BufReader::new(file);
    let mut report: Vec<String> = Vec::new();
    let mut counter: Vec<usize> = Vec::new();
    for line in reader.lines() {
        let line = line?;
        if counter.is_empty() {
            counter = vec![0; line.len()];
        }
        for (i, c) in line.chars().enumerate() {
            if c == '1' {
                counter[i] += 1;
            }
        }
        report.push(line.to_string());
    }
    let mut gamma = 0;
    let mut epsilon = 0;
    for cnt in counter.iter() {
        gamma <<= 1;
        epsilon <<= 1;
        if *cnt > report.len() / 2 {
            gamma += 1;
        } else {
            epsilon += 1;
        }
    }
    println!("Part 1: {}", epsilon * gamma);

    let oxygen_rating = get_rating(&mut report, true).unwrap();
    let co2_rating = get_rating(&mut report, false).unwrap();
    println!("Part 2: {}", oxygen_rating * co2_rating);

    Ok(())
}
