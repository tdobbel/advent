use std::env;
use std::fs;

fn solve(input: &[char], size: usize) -> usize {
    input
        .windows(size)
        .position(|w| {
            let mut vec = Vec::with_capacity(size);
            for x in w {
                if vec.contains(x) {
                    return false;
                }
                vec.push(*x);
            }
            true
        })
        .map(|x| x + size)
        .unwrap()
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let line = fs::read_to_string(args).expect("Cannot read input file");
    let chars: Vec<char> = line.chars().collect();
    println!("Part 1: {}", solve(&chars, 4));
    println!("Part 2: {}", solve(&chars, 14));
}
