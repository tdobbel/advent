use anyhow::Result;
use std::env;
use std::fs;

pub fn is_invalid(number: &str, n_div: usize) -> bool {
    let n_digit = number.len();
    if !n_digit.is_multiple_of(n_div) {
        return false;
    }
    let size = n_digit / n_div;
    let slices: Vec<&str> = (0..n_div)
        .map(|i| &number[i * size..(i + 1) * size])
        .collect();
    for i in 0..size {
        let digit = slices[0].chars().nth(i).unwrap();
        for slice in slices.iter().skip(1) {
            let other_digit = slice.chars().nth(i).unwrap();
            if digit != other_digit {
                return false;
            }
        }
    }
    true
}

pub fn solve_part1(start: u64, stop: u64, sol: &mut u64) {
    let mut n = start;
    while n <= stop {
        let nstr = format!("{n}");
        let n_digit = nstr.len();
        if n_digit % 2 == 1 {
            n = 10_u64.pow(n_digit as u32);
            continue;
        }
        if is_invalid(&nstr, 2) {
            *sol += n;
        }
        n += 1;
    }
}

pub fn solve_part2(start: u64, stop: u64, sol: &mut u64) {
    for n in start..=stop {
        let nstr = format!("{n}");
        let n_digit = nstr.len();
        for n_div in 2..=n_digit {
            if is_invalid(&nstr, n_div) {
                *sol += n;
                break;
            }
        }
    }
}

fn main() -> Result<()> {
    let args = env::args().nth(1).expect("Please provide an input file");
    let line = fs::read_to_string(args)?;
    let line = line.trim();
    let mut part1 = 0;
    let mut part2 = 0;
    for range in line.split(",") {
        let start_stop: Vec<u64> = range.split("-").map(|x| x.parse().unwrap()).collect();
        solve_part1(start_stop[0], start_stop[1], &mut part1);
        solve_part2(start_stop[0], start_stop[1], &mut part2);
    }
    println!("Part 1: {part1}");
    println!("Part 2: {part2}");
    Ok(())
}
