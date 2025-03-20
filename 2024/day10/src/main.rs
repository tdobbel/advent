use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::collections::HashMap;


fn find_trails(topo: &[usize], ncols: usize, index: usize, tails: &mut HashMap<usize,usize>) {
    if topo[index] == 9 {
        *tails.entry(index).or_insert(0) += 1;
        return;
    }
    let y0 = (index / ncols) as i32;
    let x0 = (index % ncols) as i32;
    let ncols_ = ncols as i32;
    let nrows = topo.len() as i32 / ncols_;
    for (dx, dy) in [(-1,0), (1,0), (0,-1), (0,1)].iter() {
        let x = x0 + dx;
        let y = y0 + dy;
        if x < 0 || x >= ncols_ || y < 0 || y >= nrows {
            continue;
        }
        let index_ = (y * ncols_ + x) as usize;
        if topo[index_] == topo[index] + 1 {
            find_trails(topo, ncols, index_, tails);
        }
    }
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut ncols: usize = 0;
    let mut topo: Vec<usize> = Vec::new();
    let mut seeds: Vec<usize> = Vec::new();
    for (i, line) in reader.lines().enumerate() {
        let line = line.unwrap();
        ncols = line.len();
        for (j, c) in line.chars().enumerate() {
            let value = c.to_digit(10).unwrap() as usize;
            topo.push(value);
            if value == 0 {
                seeds.push(i*ncols+j);
            }
        }
    }
    let mut totalscore1: usize = 0;
    let mut totalscore2: usize = 0;
    let mut tails: HashMap<usize,usize>;
    for seed in seeds.iter() {
        tails = HashMap::new();
        find_trails(&topo, ncols, *seed, &mut tails);
        totalscore2 += tails.values().sum::<usize>();
        totalscore1 += tails.len(); 
    }
    println!("Total score part 1: {}", totalscore1);
    println!("Total score part 2: {}", totalscore2);
}
