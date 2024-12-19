use std::fs::File;
use std::io::{BufRead, BufReader};
use std::cmp::{max,min};
use std::collections::HashSet;
use std::env;

fn ispossible(design: &str, patterns: &Vec<Vec<&str>>, nmax: &usize) -> bool {
    if design.len() == 0 {
        return true;
    }
    for n in 0..min(*nmax,design.len()) {
        let part = &design[..n+1];
        if !patterns[n].contains(&part) {
            continue;
        }
        if ispossible(&design[n+1..], patterns, nmax) {
            return true;
        }
    }
    false
}

fn find_all_ways(design: &str, patterns: &Vec<Vec<&str>>, nmax: &usize, prev: String, ways: &mut Vec<String>) {
    if design.len() == 0 {
        ways.push(prev);
        return
    }
    for n in 1..min(*nmax+1,design.len()+1) {
        let part = &design[..n];
        for towel in patterns[n-1].iter() {
            if part == *towel {
                let seq: String;
                if prev.len() == 0 {
                    seq = towel.to_string();
                }
                else {
                    seq = format!("{},{}", prev, towel);
                }
                find_all_ways(&design[n..], patterns, nmax, seq, ways)
            }
        }
    }
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
    let mut n_ways = 0;
    while let Some(line) = lines.next() {
        let line = line.unwrap();
        if line.len() == 0 {
            continue;
        }
        if ispossible_split(&line, &sorted_towels, &nmax) {
            n_possible += 1;
        }
        let prev = String::from("");
        let mut ways = Vec::<String>::new();
        find_all_ways(&line, &sorted_towels, &nmax, prev, &mut ways);
        n_ways += ways.iter().collect::<HashSet<_>>().len();
    }
    println!("{} designs are possible", n_possible);
    println!("{} possible ways", n_ways);
}
