use std::cmp::min;
use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

fn find_symmetry(grid: &[Vec<char>]) -> Option<usize> {
    let mut xs: Vec<usize> = (1..grid[0].len()).collect();
    for row in grid.iter() {
        let mut x_next = vec![];
        for x in xs.iter() {
            let mut ok = true;
            let n = min(*x, row.len() - *x);
            for i in 0..n {
                if row[x - i - 1] != row[x + i] {
                    ok = false;
                    break;
                }
            }
            if ok {
                x_next.push(*x);
            }
        }
        xs = x_next;
        if xs.is_empty() {
            return None;
        }
    }
    if xs.len() == 1 {
        Some(xs[0])
    } else {
        None
    }
}

fn find_symmetry_smudge(grid: &[Vec<char>]) -> Option<usize> {
    let mut xs: Vec<usize> = (1..grid[0].len()).collect();
    let mut nwrong: HashMap<usize, usize> = HashMap::new();
    for row in grid.iter() {
        let mut x_next = vec![];
        for x in xs.iter() {
            let n = min(*x, row.len() - *x);
            let nerr = nwrong.entry(*x).or_insert(0);
            for i in 0..n {
                if row[x - i - 1] != row[x + i] {
                    *nerr += 1;
                }
                if *nerr > 1 {
                    break;
                }
            }
            if *nerr <= 1 {
                x_next.push(*x);
            }
        }
        xs = x_next;
        if xs.is_empty() {
            return None;
        }
    }
    for (k, v) in nwrong.iter() {
        if *v == 1 {
            return Some(*k);
        }
    }
    None
}

fn transpose(grid: &[Vec<char>]) -> Vec<Vec<char>> {
    let mut new_grid = vec![];
    for i in 0..grid[0].len() {
        let mut row = vec![];
        for j in 0..grid.len() {
            row.push(grid[j][i]);
        }
        new_grid.push(row);
    }
    new_grid
}

fn part1(grid: &[Vec<char>]) -> usize {
    match find_symmetry(grid) {
        Some(i) => i,
        None => {
            let transposed = transpose(grid);
            match find_symmetry(&transposed) {
                Some(i) => i * 100,
                None => panic!("No symmetry found"),
            }
        }
    }
}

fn part2(grid: &[Vec<char>]) -> usize {
    match find_symmetry_smudge(grid) {
        Some(i) => i,
        None => {
            let transposed = transpose(grid);
            match find_symmetry_smudge(&transposed) {
                Some(i) => i * 100,
                None => panic!("No symmetry found"),
            }
        }
    }
}

fn main() {
    let args = env::args().nth(1).expect("Please provide an input file");
    let file = File::open(args).unwrap();
    let reader = BufReader::new(file);
    let mut grid: Vec<Vec<char>> = vec![];
    let mut total1 = 0;
    let mut total2 = 0;
    for line in reader.lines() {
        let line = line.unwrap();
        if line.is_empty() {
            total1 += part1(&grid);
            total2 += part2(&grid);
            grid = vec![];
            continue;
        }
        grid.push(line.chars().collect());
    }
    if !grid.is_empty() {
        total1 += part1(&grid);
        total2 += part2(&grid);
    }
    println!("Part 1: {}", total1);
    println!("Part 2: {}", total2);
}
