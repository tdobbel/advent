use std::fs::File;
use std::io::{BufRead, BufReader};
use std::env;

fn main() {
    let args: Vec<String> = env::args().collect();
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut locks = Vec::<Vec<i32>>::new();
    let mut keys = Vec::<Vec<i32>>::new();
    let mut lines = reader.lines();
    let mut cntr = 0;
    let mut cols: Vec<i32> = vec![0; 5];
    let mut islock = false;
    while let Some(line) = lines.next() {
        let line = line.unwrap();
        if line.len() == 0 {
            continue;
        }
        if cntr == 0  {
            islock = line.contains("#");
        }
        cntr += 1;
        for (i,c) in line.chars().enumerate() {
            if c == '#' {
                cols[i] += 1;
            }
        }
        if cntr == 7 {
            if islock {
                locks.push(cols.iter().map(|x| *x-1).collect());
            }
            else {
                keys.push(cols.iter().map(|x| *x-1).collect());
            }
            cols = vec![0; 5];
            cntr = 0;
        }
    }
    let mut result = 0;
    for lock in locks.iter() {
        for key in keys.iter() {
            let mut matching = true;
            for i in 0..5 {
                if lock[i] + key[i] > 5 {
                    matching = false;
                    break;
                }
            }
            if matching {
                result += 1;
            }
        }
    }
    println!("{}", result);
}
