use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn unfold(puzzle: &[char], target: &[usize]) -> (Vec<char>, Vec<usize>) {
    let mut new_puzzle = Vec::<char>::new();
    let mut new_target = Vec::<usize>::new();
    for i in 0..5 {
        new_puzzle.extend(puzzle);
        if i < 4 {
            new_puzzle.push('?');
        }
        new_target.extend(target);
    }
    (new_puzzle, new_target)
}

fn count_possibilities(
    puzzle: &[char],
    target: &[usize],
    memory: &mut HashMap<(Vec<char>, Vec<usize>), usize>,
    value: usize,
) -> usize {
    if puzzle.is_empty() {
        if value == 0 {
            return if target.is_empty() { 1 } else { 0 };
        } else if target.len() != 1 {
            return 0;
        } else {
            return if value == target[0] { 1 } else { 0 };
        }
    }
    if target.is_empty() && puzzle.contains(&'#') {
        return 0;
    }
    let v = puzzle.first().unwrap();
    match v {
        '?' => {
            let mut v1 = puzzle.to_owned();
            v1[0] = '#';
            let mut v2 = puzzle.to_owned();
            v2[0] = '.';
            count_possibilities(&v1, target, memory, value)
                + count_possibilities(&v2, target, memory, value)
        }
        '.' => {
            if value == 0 {
                match memory.get(&(puzzle[1..].to_vec(), target.to_vec())) {
                    Some(v) => return *v,
                    None => {
                        let value = count_possibilities(&puzzle[1..], target, memory, 0);
                        memory.insert((puzzle[1..].to_vec(), target.to_vec()), value);
                        return value;
                    }
                }
            }
            match target.first() {
                Some(v) => {
                    if value != *v {
                        return 0;
                    }
                }
                None => return 0,
            }
            match memory.get(&(puzzle[1..].to_vec(), target[1..].to_vec())) {
                Some(v) => *v,
                None => {
                    let value = count_possibilities(&puzzle[1..], &target[1..], memory, 0);
                    memory.insert((puzzle[1..].to_vec(), target[1..].to_vec()), value);
                    value
                }
            }
        }
        '#' => count_possibilities(&puzzle[1..], target, memory, value + 1),
        _ => {
            panic!("Unexpected character '{}'", v);
        }
    }
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut total1 = 0;
    let mut total2 = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        let mut splitted = line.split_whitespace();
        let puzzle: Vec<char> = splitted.next().unwrap().chars().collect();
        let target: Vec<usize> = splitted
            .next()
            .unwrap()
            .split(',')
            .map(|x| x.parse().unwrap())
            .collect();
        let mut memory = HashMap::new();
        total1 += count_possibilities(&puzzle, &target, &mut memory, 0);
        let (puzzle, target) = unfold(&puzzle, &target);
        total2 += count_possibilities(&puzzle, &target, &mut memory, 0)
    }
    println!("Part 1: {}", total1);
    println!("Part 2: {}", total2);
}
