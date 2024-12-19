use std::fs::File;
use std::io::{BufRead, BufReader};
use std::cmp::{max,min};
use std::env;

fn ispossible(design: &str, patterns: &Vec<Vec<&str>>, nmax: &usize) -> bool {
    if design.len() <= patterns.len() && patterns[design.len()-1].contains(&design) {
        return true;
    }
    for n in 1..min(*nmax+1,design.len()) {
        let part = &design[..n];
        if !patterns[n-1].contains(&part) {
            continue;
        }
        if ispossible(&design[n..], patterns, nmax) {
            return true;
        }
    }
    false
}

fn ispossible_split(design: &str, patterns: &Vec<Vec<&str>>, nmax: &usize) -> bool {
    if design.len() < 2*nmax {
        return ispossible(design, patterns, nmax);
    }
    let mid = design.len() / 2;
    let start = mid - nmax/2;
    let stop = mid + nmax/2;
    for pivot in start..stop+1 {
        if ispossible_split(&design[..pivot], patterns, nmax) && ispossible_split(&design[pivot..], patterns, nmax) {
            return true;
        }
    }
    false
}

fn main() {
    let args: Vec<String> = env::args().collect();
    assert_eq!(args.len(), 2);
    let file = File::open(&args[1]).unwrap();
    let reader = BufReader::new(file);
    let mut lines = reader.lines();
    let line = lines.next().unwrap().unwrap();
    let towels = line.split(", ").collect::<Vec<&str>>();
    let mut nmax: usize = 0;
    for towel in towels.iter() {
        nmax = max(nmax, towel.len());
    }
    let mut sorted_towels = Vec::<Vec<&str>>::new();
    for n in 1..nmax+1 {
        let selected = towels.clone().into_iter().filter(|&towel| towel.len() == n).collect::<Vec<&str>>();
        sorted_towels.push(selected);
    }
    let mut n_possible = 0;
    while let Some(line) = lines.next() {
        let line = line.unwrap();
        if line.len() == 0 {
            continue;
        }
        if ispossible_split(&line, &sorted_towels, &nmax) {
            n_possible += 1;
        }
    }
    println!("{} designs are possible", n_possible);
}
