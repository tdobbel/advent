use std::fs::File;
use std::io::{BufRead, BufReader};
use std::cmp::{max,min};
use std::env;

#[allow(dead_code)]
fn ispossible(design: &str, patterns: &Vec<Vec<&str>>, nmax: &usize) -> bool {
    if design.is_empty() {
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

#[allow(dead_code)]
fn find_all_ways(design: &str, patterns: &Vec<Vec<&str>>, nmax: &usize, prev: String, ways: &mut Vec<String>) {
    if design.is_empty() {
        ways.push(prev);
        return
    }
    for n in 1..min(*nmax+1,design.len()+1) {
        let part = &design[..n];
        for towel in patterns[n-1].iter() {
            if part == *towel {
                let seq: String = if prev.is_empty(){
                    towel.to_string()
                }
                else {
                    format!("{},{}", prev, towel)
                };
                find_all_ways(&design[n..], patterns, nmax, seq, ways)
            }
        }
    }
}

#[allow(dead_code)]
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

fn count_possibilities(design: &str, patterns: &[Vec<&str>], nmax: &usize) -> u64 {
    let mut counter: Vec<u64> = vec![0; design.len()];
    for i in 0..design.len() {
        let stop = i+1;
        for j in 0..min(*nmax,i+1) {
            let start = i-j;
            if patterns[j].contains(&&design[start..stop]) {
                if start > 0 {
                    counter[i] += counter[start-1]
                }
                else {
                    counter[i] += 1;
                }
            }
        }

    }
    counter[counter.len()-1]
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
    let mut n_ways: u64 = 0;
    for line in lines {
        let line = line.unwrap();
        if line.len() == 0 {
            continue;
        }
        //let possible = ispossible_split(&line, &sorted_towels, &nmax);
        let n_sol = count_possibilities(&line, &sorted_towels, &nmax);
        if n_sol > 0 {
            n_possible += 1;
            n_ways += n_sol;
        }
    }
    println!("{} designs are possible", n_possible);
    println!("There are {} ways to obtain these designs", n_ways);
}
